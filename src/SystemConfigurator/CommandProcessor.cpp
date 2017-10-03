/*
Copyright 2017 Microsoft
Permission is hereby granted, free of charge, to any person obtaining a copy of this software 
and associated documentation files (the "Software"), to deal in the Software without restriction, 
including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, 
subject to the following conditions:

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT 
LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH 
THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "stdafx.h"
#include <fstream>
#include "..\SharedUtilities\Logger.h"
#include "..\SharedUtilities\DMRequest.h"
#include "..\SharedUtilities\SecurityAttributes.h"
#include "CSPs\MdmProvision.h"
#include "CSPs\CertificateInfo.h"
#include "CSPs\CertificateManagement.h"
#include "CSPs\CustomDeviceUiCsp.h"
#include "CSPs\DeviceHealthAttestationCSP.h"
#include "CSPs\EnterpriseModernAppManagementCSP.h"
#include "CSPs\DiagnosticLogCSP.h"
#include "CSPs\RebootCSP.h"
#include "CSPs\WifiCsp.h"
#include "CSPs\WindowsUpdatePolicyCSP.h"
#include "AppCfg.h"
#include "DMStorage.h"
#include "TimeCfg.h"
#include "TimeService.h"
#include "TpmSupport.h"
#include "WindowsTelemetry.h"

#include "Models\AllModels.h"

#include "SystemConfiguratorProxyServer\SystemConfiguratorProxy.h"

using namespace Microsoft::Devices::Management::Message;
using namespace std;
using namespace Windows::Data::Json;
using namespace Windows::Foundation::Collections;

//
// This registry key/value pair needs to stay in sync with the one found here:
//    https://github.com/ms-iot/iot-azure-dps-client/blob/master/src/IotDpsClient/DPSServiceImpl.cpp
//
const wchar_t* TpmSlotRegistrySubKey = L"SYSTEM\\CurrentControlSet\\Services\\iotdpsclient\\parameters";
const wchar_t* TpmSlotPropertyName = L"tpm_slot";

StringResponse^ ReportError(const string& context, const DMException& e)
{
    string debugMessage = "Error: " + context;
    TRACEP(debugMessage.c_str(), e.what());
    auto errorMessageCStr = Utils::MultibyteToWide(e.what());
    auto responseMessage = ref new String(errorMessageCStr.c_str(), static_cast<unsigned int>(errorMessageCStr.length()));
    return ref new StringResponse(ResponseStatus::Failure, responseMessage, DMMessageKind::ErrorResponse);
}

IResponse^ HandleExitDM(IRequest^ request)
{
    TRACE(__FUNCTION__);

    TRACE(L"Disconnecting RPC listener...");
    SystemConfiguratorProxyDisconnect();

    return ref new StatusCodeResponse(ResponseStatus::Success, request->Tag);
}

IResponse^ HandleFactoryReset(IRequest^ request)
{
    TRACE(__FUNCTION__);

    auto resetRequest = dynamic_cast<FactoryResetRequest^>(request);
    TRACEP(L"clearTPM = ", (resetRequest->clearTPM ? L"true" : L"false"));
    TRACEP(L"recoveryPartitionGUID = ", resetRequest->recoveryPartitionGUID->Data());

    // Clear the TPM if requested...
    if (resetRequest->clearTPM)
    {
        Tpm::ClearTPM();
    }

    // Schedule the recovery...
    unsigned long returnCode = 0;
    string output;
    wstring command = Utils::GetSystemRootFolder() + L"\\bcdedit.exe  /set {bootmgr} bootsequence {" + resetRequest->recoveryPartitionGUID->Data() + L"}";
    TRACE(command.c_str());
    Utils::LaunchProcess(command, returnCode, output);
    if (returnCode != 0)
    {
        throw DMExceptionWithErrorCode("Error: ApplyUpdate.exe returned an error code.", returnCode);
    }

    // Reboot the device...
    RebootCSP::ExecRebootNow(Utils::GetCurrentDateTimeString());

    return ref new StatusCodeResponse(ResponseStatus::Success, request->Tag);
}

IResponse^ HandleGetWindowsTelemetry(IRequest^ request)
{
    TRACE(__FUNCTION__);
    return WindowsTelemetry::Get();
}

IResponse^ HandleSetWindowsTelemetry(IRequest^ request)
{
    TRACE(__FUNCTION__);
    WindowsTelemetry::Set(dynamic_cast<SetWindowsTelemetryRequest^>(request));
    return ref new StatusCodeResponse(ResponseStatus::Success, request->Tag);
}

IResponse^ HandleGetDeviceInfo(IRequest^ request)
{
    TRACE(__FUNCTION__);

    // ToDo: We need an efficient way of communicating details about errors for each of the various settings.

    wstring id = L"<error>";
    wstring manufacturer = L"<error>";
    wstring model = L"<error>";
    wstring dmVer = L"<error>";
    wstring lang = L"<error>";

    wstring type = L"<error>";
    wstring oem = L"<error>";
    wstring hwVer = L"<error>";
    wstring fwVer = L"<error>";
    wstring osVer = L"<error>";

    wstring platform = L"<error>";
    wstring processorType = L"<error>";
    wstring radioSwVer = L"<error>";
    wstring displayResolution = L"<error>";
    wstring commercializationOperator = L"<error>";

    wstring processorArchitecture = L"<error>";
    wstring name = L"<error>";
    wstring totalStorage = L"<error>";
    wstring totalMemory = L"<error>";
    wstring secureBootState = L"<error>";

    wstring osEdition = L"<error>";
    wstring batteryStatus = L"<error>";
    wstring batteryRemaining = L"<error>";
    wstring batteryRuntime = L"<error>";

    MdmProvision::TryGetString(L"./DevInfo/DevId", id);
    MdmProvision::TryGetString(L"./DevInfo/Man", manufacturer);
    MdmProvision::TryGetString(L"./DevInfo/Mod", model);
    MdmProvision::TryGetString(L"./DevInfo/DmV", dmVer);
    MdmProvision::TryGetString(L"./DevInfo/Lang", lang);

    MdmProvision::TryGetString(L"./DevDetail/DevTyp", type);
    MdmProvision::TryGetString(L"./DevDetail/OEM", oem);
    MdmProvision::TryGetString(L"./DevDetail/HwV", hwVer);
    MdmProvision::TryGetString(L"./DevDetail/FwV", fwVer);
    MdmProvision::TryGetString(L"./DevDetail/SwV", osVer);

    MdmProvision::TryGetString(L"./DevDetail/Ext/Microsoft/OSPlatform", platform);
    MdmProvision::TryGetString(L"./DevDetail/Ext/Microsoft/ProcessorType", processorType);
    MdmProvision::TryGetString(L"./DevDetail/Ext/Microsoft/RadioSwV", radioSwVer);
    MdmProvision::TryGetString(L"./DevDetail/Ext/Microsoft/Resolution", displayResolution);
    MdmProvision::TryGetString(L"./DevDetail/Ext/Microsoft/CommercializationOperator", commercializationOperator);

    MdmProvision::TryGetString(L"./DevDetail/Ext/Microsoft/ProcessorArchitecture", processorArchitecture);
    MdmProvision::TryGetString(L"./DevDetail/Ext/Microsoft/DeviceName", name);

    ULARGE_INTEGER sizeInBytes;
    if (GetDiskFreeSpaceEx(L"c:\\", NULL, &sizeInBytes, NULL))
    {
        unsigned int sizeInMB = static_cast<unsigned int>(sizeInBytes.QuadPart / 1024 / 1024);
        totalStorage = Utils::MultibyteToWide(to_string(sizeInMB).c_str());
    }
    else
    {
        totalStorage = wstring(L"error: ") + Utils::MultibyteToWide(to_string(GetLastError()).c_str());
        false;
    }

    MdmProvision::TryGetNumber<unsigned int>(L"./DevDetail/Ext/Microsoft/TotalRAM", totalMemory);
    MdmProvision::TryGetNumber<unsigned int>(L"./Vendor/MSFT/DeviceStatus/SecureBootState", secureBootState);
    MdmProvision::TryGetString(L"./Vendor/MSFT/DeviceStatus/OS/Edition", osEdition);
    MdmProvision::TryGetNumber<unsigned int>(L"./Vendor/MSFT/DeviceStatus/Battery/Status", batteryStatus);
    MdmProvision::TryGetNumber<char>(L"./Vendor/MSFT/DeviceStatus/Battery/EstimatedChargeRemaining", batteryRemaining);
    MdmProvision::TryGetNumber<int>(L"./Vendor/MSFT/DeviceStatus/Battery/EstimatedRuntime", batteryRuntime);

    GetDeviceInfoResponse^ getDeviceInfoResponse = ref new GetDeviceInfoResponse(ResponseStatus::Success);

    getDeviceInfoResponse->id = ref new String(id.c_str());
    getDeviceInfoResponse->manufacturer = ref new String(manufacturer.c_str());
    getDeviceInfoResponse->model = ref new String(model.c_str());
    getDeviceInfoResponse->dmVer = ref new String(dmVer.c_str());
    getDeviceInfoResponse->lang = ref new String(lang.c_str());

    getDeviceInfoResponse->type = ref new String(type.c_str());
    getDeviceInfoResponse->oem = ref new String(oem.c_str());
    getDeviceInfoResponse->hwVer = ref new String(hwVer.c_str());
    getDeviceInfoResponse->fwVer = ref new String(fwVer.c_str());
    getDeviceInfoResponse->osVer = ref new String(osVer.c_str());

    getDeviceInfoResponse->platform = ref new String(platform.c_str());
    getDeviceInfoResponse->processorType = ref new String(processorType.c_str());
    getDeviceInfoResponse->radioSwVer = ref new String(radioSwVer.c_str());
    getDeviceInfoResponse->displayResolution = ref new String(displayResolution.c_str());
    getDeviceInfoResponse->commercializationOperator = ref new String(commercializationOperator.c_str());

    getDeviceInfoResponse->processorArchitecture = ref new String(processorArchitecture.c_str());
    getDeviceInfoResponse->name = ref new String(name.c_str());
    getDeviceInfoResponse->totalStorage = ref new String(totalStorage.c_str());
    getDeviceInfoResponse->totalMemory = ref new String(totalMemory.c_str());
    getDeviceInfoResponse->secureBootState = ref new String(secureBootState.c_str());

    getDeviceInfoResponse->osEdition = ref new String(osEdition.c_str());
    getDeviceInfoResponse->batteryStatus = ref new String(batteryStatus.c_str());
    getDeviceInfoResponse->batteryRemaining = ref new String(batteryRemaining.c_str());
    getDeviceInfoResponse->batteryRuntime = ref new String(batteryRuntime.c_str());

    return getDeviceInfoResponse;
}

IResponse^ HandleSetTimeInfo(IRequest^ request)
{
    TRACE(__FUNCTION__);

    auto setTimeInfoRequest = dynamic_cast<SetTimeInfoRequest^>(request);
    TimeCfg::Set(setTimeInfoRequest);
    return ref new StatusCodeResponse(ResponseStatus::Success, request->Tag);
}

IResponse^ HandleGetTimeService(IRequest^ request)
{
    TRACE(__FUNCTION__);

    auto timeServiceRequest = dynamic_cast<GetTimeServiceRequest^>(request);
    TimeServiceData^ data = TimeService::GetState();
    return ref new GetTimeServiceResponse(ResponseStatus::Success, data);
}

IResponse^ HandleSetTimeService(IRequest^ request)
{
    TRACE(__FUNCTION__);

    auto timeServiceRequest = dynamic_cast<SetTimeServiceRequest^>(request);
    TimeService::SetState(timeServiceRequest->data);
    return ref new StatusCodeResponse(ResponseStatus::Success, request->Tag);
}

IResponse^ HandleGetCertificateConfiguration(IRequest^ request)
{
    TRACE(__FUNCTION__);

    wstring certificateStore_CA_System = L"<error>";
    wstring certificateStore_Root_System = L"<error>";
    wstring certificateStore_My_User = L"<error>";
    wstring certificateStore_My_System = L"<error>";
    wstring rootCATrustedCertificates_Root = L"<error>";
    wstring rootCATrustedCertificates_CA = L"<error>";
    wstring rootCATrustedCertificates_TrustedPublisher = L"<error>";
    wstring rootCATrustedCertificates_TrustedPeople = L"<error>";

    MdmProvision::TryGetString(L"./Vendor/MSFT/CertificateStore/CA/System", certificateStore_CA_System);
    MdmProvision::TryGetString(L"./Vendor/MSFT/CertificateStore/Root/System", certificateStore_Root_System);
    MdmProvision::TryGetString(L"./Vendor/MSFT/CertificateStore/My/User", certificateStore_My_User);
    MdmProvision::TryGetString(L"./Vendor/MSFT/CertificateStore/My/System", certificateStore_My_System);
    MdmProvision::TryGetString(L"./Device/Vendor/MSFT/RootCATrustedCertificates/Root", rootCATrustedCertificates_Root);
    MdmProvision::TryGetString(L"./Device/Vendor/MSFT/RootCATrustedCertificates/CA", rootCATrustedCertificates_CA);
    MdmProvision::TryGetString(L"./Device/Vendor/MSFT/RootCATrustedCertificates/TrustedPublisher", rootCATrustedCertificates_TrustedPublisher);
    MdmProvision::TryGetString(L"./Device/Vendor/MSFT/RootCATrustedCertificates/TrustedPeople", rootCATrustedCertificates_TrustedPeople);

    CertificateConfiguration^ configuration = ref new CertificateConfiguration();

    configuration->certificateStore_CA_System = ref new String(certificateStore_CA_System.c_str());
    configuration->certificateStore_Root_System = ref new String(certificateStore_Root_System.c_str());
    configuration->certificateStore_My_User = ref new String(certificateStore_My_User.c_str());
    configuration->certificateStore_My_System = ref new String(certificateStore_My_System.c_str());
    configuration->rootCATrustedCertificates_Root = ref new String(rootCATrustedCertificates_Root.c_str());
    configuration->rootCATrustedCertificates_CA = ref new String(rootCATrustedCertificates_CA.c_str());
    configuration->rootCATrustedCertificates_TrustedPublisher = ref new String(rootCATrustedCertificates_TrustedPublisher.c_str());
    configuration->rootCATrustedCertificates_TrustedPeople = ref new String(rootCATrustedCertificates_TrustedPeople.c_str());

    return ref new GetCertificateConfigurationResponse(ResponseStatus::Success, configuration);
}

IResponse^ HandleSetCertificateConfiguration(IRequest^ request)
{
    TRACE(__FUNCTION__);

    auto setCertificateConfigurationRequest = dynamic_cast<SetCertificateConfigurationRequest^>(request);
    CertificateConfiguration^ configuration = setCertificateConfigurationRequest->configuration;

    CertificateManagement::SyncCertificates(L"./Vendor/MSFT/CertificateStore/CA/System", configuration->certificateStore_CA_System->Data());
    CertificateManagement::SyncCertificates(L"./Vendor/MSFT/CertificateStore/Root/System", configuration->certificateStore_Root_System->Data());
    CertificateManagement::SyncCertificates(L"./Vendor/MSFT/CertificateStore/My/User", configuration->certificateStore_My_User->Data());
    CertificateManagement::SyncCertificates(L"./Vendor/MSFT/CertificateStore/My/System", configuration->certificateStore_My_System->Data());

    CertificateManagement::SyncCertificates(L"./Device/Vendor/MSFT/RootCATrustedCertificates/Root", configuration->rootCATrustedCertificates_Root->Data());
    CertificateManagement::SyncCertificates(L"./Device/Vendor/MSFT/RootCATrustedCertificates/CA", configuration->rootCATrustedCertificates_CA->Data());
    CertificateManagement::SyncCertificates(L"./Device/Vendor/MSFT/RootCATrustedCertificates/TrustedPublisher", configuration->rootCATrustedCertificates_TrustedPublisher->Data());
    CertificateManagement::SyncCertificates(L"./Device/Vendor/MSFT/RootCATrustedCertificates/TrustedPeople", configuration->rootCATrustedCertificates_TrustedPeople->Data());

    return ref new StatusCodeResponse(ResponseStatus::Success, request->Tag);
}

IResponse^ HandleGetCertificateDetails(IRequest^ request)
{
    TRACE(__FUNCTION__);

    auto getCertificateDetailsRequest = dynamic_cast<GetCertificateDetailsRequest^>(request);
    wstring path = getCertificateDetailsRequest->path->Data();
    wstring hash = getCertificateDetailsRequest->hash->Data();
    TRACEP(L"path = ", path.c_str());
    TRACEP(L"hash = ", hash.c_str());
    CertificateInfo certificateInfo(path + L"/" + hash);

    GetCertificateDetailsResponse^ getCertificateDetailsResponse = ref new GetCertificateDetailsResponse(ResponseStatus::Success);
    getCertificateDetailsResponse->issuedBy = ref new String(certificateInfo.GetIssuedBy().c_str());
    getCertificateDetailsResponse->issuedTo = ref new String(certificateInfo.GetIssuedTo().c_str());
    getCertificateDetailsResponse->validFrom = ref new String(certificateInfo.GetValidFrom().c_str());
    getCertificateDetailsResponse->validTo = ref new String(certificateInfo.GetValidTo().c_str());
    getCertificateDetailsResponse->base64Encoding = ref new String(certificateInfo.GetCertificateInBase64().c_str());
    getCertificateDetailsResponse->templateName = ref new String(certificateInfo.GetTemplateName().c_str());

    return getCertificateDetailsResponse;
}

IResponse^ HandleGetWifiConfiguration(IRequest^ request)
{
    TRACE(__FUNCTION__);

    auto profiles = WifiCSP::GetProfiles();

    auto configuration = ref new WifiConfiguration();
    configuration->ReportToDeviceTwin = ref new Platform::String(L"yes");
    for each (auto profile in profiles)
    {
        auto profileConfig = ref new WifiProfileConfiguration();
        profileConfig->Name = ref new Platform::String(profile.c_str());
        TRACEP(L"Wifi profile found: ", profileConfig->Name->Data());
        configuration->Profiles->Append(profileConfig);
    }
    return ref new GetWifiConfigurationResponse(ResponseStatus::Success, configuration);
}

IResponse^ HandleSetWifiConfiguration(IRequest^ request)
{
    TRACE(__FUNCTION__);

    auto wifiRequest = dynamic_cast<SetWifiConfigurationRequest^>(request);
    auto configuration = wifiRequest->Configuration;

    if (configuration != nullptr)
    {
        for each (auto profile in configuration->Profiles)
        {
            std::wstring profileName = profile->Name->Data();
            TRACEP(L"DMCommand::HandleSetWifiConfiguration handle profile: ", profileName);
            TRACEP("DMCommand::HandleSetWifiConfiguration uninstall? ", profile->Uninstall);
            if (profile->Uninstall)
            {
                WifiCSP::DeleteProfile(profileName);
            }
            else
            {
                std::wstring profileXml = profile->Xml->Data();
                WifiCSP::AddProfile(profileName, profileXml);
            }
        }
    }

    return ref new StatusCodeResponse(ResponseStatus::Success, request->Tag);
}

IResponse^ HandleGetWifiDetails(IRequest^ request)
{
    TRACE(__FUNCTION__);

    auto getWifiDetailsRequest = dynamic_cast<GetWifiDetailsRequest^>(request);
    wstring profileName = getWifiDetailsRequest->profileName->Data();
    auto xml = WifiCSP::GetProfile(profileName);

    GetWifiDetailsResponse^ getWifiDetailsResponse = ref new GetWifiDetailsResponse(ResponseStatus::Success);
    getWifiDetailsResponse->Name = getWifiDetailsRequest->profileName;
    getWifiDetailsResponse->Xml = ref new Platform::String(xml.c_str());

    return getWifiDetailsResponse;
}

IResponse^ HandleGetRebootInfo(IRequest^ request)
{
    TRACE(__FUNCTION__);

    GetRebootInfoResponse^ response = ref new GetRebootInfoResponse(ResponseStatus::Success);
    response->singleRebootTime = ref new String(RebootCSP::GetSingleScheduleTime().data());
    response->dailyRebootTime = ref new String(RebootCSP::GetDailyScheduleTime().data());
    response->lastBootTime = ref new String(RebootCSP::GetLastRebootTime().data());
    response->lastRebootCmdTime = ref new String(RebootCSP::GetLastRebootCmdTime().data());
    return response;
}

IResponse^ HandleSetRebootInfo(IRequest^ request)
{
    TRACE(__FUNCTION__);

    auto setRebootInfoRequest = dynamic_cast<SetRebootInfoRequest^>(request);
    RebootCSP::SetSingleScheduleTime(setRebootInfoRequest->singleRebootTime->Data());
    RebootCSP::SetDailyScheduleTime(setRebootInfoRequest->dailyRebootTime->Data());
    return ref new StatusCodeResponse(ResponseStatus::Success, request->Tag);
}

IResponse^ HandleGetTimeInfo(IRequest^ request)
{
    TRACE(__FUNCTION__);
    return TimeCfg::Get();
}

IResponse^ HandleImmediateReboot(IRequest^ request)
{
    TRACE(__FUNCTION__);

    auto immediateRebootRequest = dynamic_cast<ImmediateRebootRequest^>(request);
    RebootCSP::ExecRebootNow(immediateRebootRequest->lastRebootCmdTime->Data());
    return ref new StatusCodeResponse(ResponseStatus::Success, request->Tag);
}

IResponse^ HandleCheckUpdates(IRequest^ request)
{
    return ref new CheckForUpdatesResponse(ResponseStatus::Success, true);
}

void SetAppStartUpType(const wstring& pkgFamilyName, StartUpType startUpType)
{
    TRACE(__FUNCTION__);
    TRACEP(L"Package Family Name: ", pkgFamilyName.c_str());

    switch (startUpType)
    {
    case StartUpType::None:
        if (CustomDeviceUiCSP::IsBackground(pkgFamilyName))
        {
            CustomDeviceUiCSP::RemoveBackgroundApplicationAsStartupApp(pkgFamilyName);
        }

        if (CustomDeviceUiCSP::IsForeground(pkgFamilyName))
        {
            // If code ever reaches here, it means that we did not process the the settings in the correct order.
            // The foreground app should always be set to a new one before the older one is set to 'none'.
            throw DMExceptionWithErrorCode("Cannot remove the app from the foreground start-up list. Set the foreground startup app to some other app first!", -1);
        }
        break;
    case StartUpType::Foreground:
        CustomDeviceUiCSP::AddAsStartupApp(pkgFamilyName, false /*!background*/);
        break;
    case StartUpType::Background:
        CustomDeviceUiCSP::AddAsStartupApp(pkgFamilyName, true /*background*/);
        break;
    }
}

StartUpType GetAppStartUpType(const wstring& pkgFamilyName)
{
    TRACE(__FUNCTION__);

    if (CustomDeviceUiCSP::IsBackground(pkgFamilyName))
    {
        return StartUpType::Background;
    }
    else if (CustomDeviceUiCSP::IsForeground(pkgFamilyName))
    {
        return StartUpType::Foreground;
    }
    return StartUpType::None;
}

IResponse^ HandleInstallApp(IRequest^ request)
{
    TRACE(__FUNCTION__);

    auto appInstallRequest = dynamic_cast<AppInstallRequest^>(request);
    auto info = appInstallRequest->data;

    std::vector<wstring> deps;
    for each (auto dep in info->Dependencies)
    {
        deps.push_back((wstring)dep->Data());
    }
    auto packageFamilyName = (wstring)info->PackageFamilyName->Data();
    auto appxPath = (wstring)info->AppxPath->Data();
    auto certFile = (wstring)info->CertFile->Data();
    auto certStore = (wstring)info->CertStore->Data();
    auto isSelfUpdate = info->IsDMSelfUpdate;

    // ToDo: Need to either fix the CSP api, or just stick with the WinRT interface.
    // EnterpriseModernAppManagementCSP::ApplicationInfo applicationInfo = EnterpriseModernAppManagementCSP::InstallApp(packageFamilyName, appxPath, deps);
    ApplicationInfo applicationInfo = AppCfg::InstallApp(packageFamilyName, appxPath, deps, certFile, certStore, isSelfUpdate);

    AppInstallResponseData^ responseData = ref new AppInstallResponseData();
    responseData->pkgFamilyName = ref new String(applicationInfo.packageFamilyName.c_str());
    responseData->name = ref new String(applicationInfo.name.c_str());
    responseData->installDate = ref new String(applicationInfo.installDate.c_str());
    responseData->version = ref new String(applicationInfo.version.c_str());
    responseData->errorCode = applicationInfo.errorCode;
    responseData->errorMessage = ref new String(applicationInfo.errorMessage.c_str());
    responseData->pending = applicationInfo.pending;

    if (!isSelfUpdate)
    {
        // Handle the startup state...
        SetAppStartUpType(packageFamilyName, info->StartUp);
        responseData->startUp = GetAppStartUpType(packageFamilyName);
    }

    return ref new AppInstallResponse(ResponseStatus::Success, responseData);
}

IResponse^ HandleUninstallApp(IRequest^ request)
{
    AppUninstallResponseData^ responseData = ref new AppUninstallResponseData();
    responseData->errorCode = 0;
    responseData->errorMessage = L"";

    auto appUninstallRequest = dynamic_cast<AppUninstallRequest^>(request);
    auto requestData = appUninstallRequest->data;
    auto packageFamilyName = (wstring)requestData->PackageFamilyName->Data();

    SetAppStartUpType(packageFamilyName, StartUpType::None);

    // ToDo: Need to either fix the CSP api, or just stick with the WinRT interface.
    // auto storeApp = info->StoreApp;
    // EnterpriseModernAppManagementCSP::UninstallApp(packageFamilyName, storeApp);
    AppCfg::UninstallApp(packageFamilyName.c_str());
    responseData->errorCode = 0;
    responseData->errorMessage = L"";
    return ref new AppUninstallResponse(ResponseStatus::Success, responseData);
}

IResponse^ HandleTransferFile(IRequest^ request)
{
    TRACE(__FUNCTION__);

    auto transferRequest = dynamic_cast<AzureFileTransferRequest^>(request);
    auto info = transferRequest->AzureFileTransferInfo;
    auto upload = info->Upload;
    auto relativeLocalPath = (wstring)info->RelativeLocalPath->Data();
    auto appLocalDataPath = (wstring)info->AppLocalDataPath->Data();

    auto localPathBase = Utils::GetDmUserFolder();
    auto localPath = localPathBase + relativeLocalPath;

    TRACEP(L"Local path     = ", localPath.c_str());
    TRACEP(L"App local path = ", appLocalDataPath.c_str());

    std::ifstream  src((upload) ? localPath : appLocalDataPath, std::ios::binary);
    std::ofstream  dst((!upload) ? localPath : appLocalDataPath, std::ios::binary);
    dst << src.rdbuf();

    return ref new StatusCodeResponse(ResponseStatus::Success, request->Tag);
}

IResponse^ HandleAppLifecycle(IRequest^ request)
{
    auto appLifecycle = dynamic_cast<AppLifecycleRequest^>(request);
    auto info = appLifecycle->AppLifecycleInfo;
    auto appId = (wstring)info->AppId->Data();
    bool start = info->Start;

    if (start) AppCfg::StartApp(appId);
    else AppCfg::StopApp(appId);
    return ref new StatusCodeResponse(ResponseStatus::Success, request->Tag);
}

IResponse^ HandleStartApp(IRequest^ request)
{
    return HandleAppLifecycle(request);
}

IResponse^ HandleStopApp(IRequest^ request)
{
    return HandleAppLifecycle(request);
}

IResponse^ HandleAddRemoveAppForStartup(StartupAppInfo^ info, DMMessageKind tag, bool add)
{
    TRACE(__FUNCTION__);

    auto pkgFamilyName = (wstring)info->AppId->Data();

    TRACEP(L"pkgFamilyName = ", pkgFamilyName.c_str());

    auto isBackgroundApp = info->IsBackgroundApplication;

    if (add)
    {
        CustomDeviceUiCSP::AddAsStartupApp(pkgFamilyName, isBackgroundApp);
    }
    else
    {
        CustomDeviceUiCSP::RemoveBackgroundApplicationAsStartupApp(pkgFamilyName);
    }
    return ref new StatusCodeResponse(ResponseStatus::Success, tag);
}

IResponse^ HandleAddStartupApp(IRequest^ request)
{
    auto startupApp = dynamic_cast<AddStartupAppRequest^>(request);
    auto info = startupApp->StartupAppInfo;
    return HandleAddRemoveAppForStartup(info, request->Tag, true);
}

IResponse^ HandleRemoveStartupApp(IRequest^ request)
{
    auto startupApp = dynamic_cast<RemoveStartupAppRequest^>(request);
    auto info = startupApp->StartupAppInfo;
    return HandleAddRemoveAppForStartup(info, request->Tag, false);
}

IResponse^ HandleGetStartupForegroundApp(IRequest^ request)
{
    TRACE(__FUNCTION__);
    auto appId = CustomDeviceUiCSP::GetStartupAppId();
    return ref new GetStartupForegroundAppResponse(ResponseStatus::Success, ref new Platform::String(appId.c_str()));
}

IResponse^ HandleListStartupBackgroundApps(IRequest^ request)
{
    TRACE(__FUNCTION__);
    auto json = CustomDeviceUiCSP::GetBackgroundTasksToLaunch();
    auto jsonArray = JsonArray::Parse(ref new Platform::String(json.c_str()));
    return ref new ListStartupBackgroundAppsResponse(ResponseStatus::Success, jsonArray);
}

IResponse^ HandleListApps(IRequest^ request)
{
    TRACE(__FUNCTION__);
    auto json = EnterpriseModernAppManagementCSP::GetInstalledApps();
    JsonObject^ jsonMap = JsonObject::Parse(ref new Platform::String(json.c_str()));

    // Inject the StartUp property.
    for each (auto pair in jsonMap)
    {
        auto pfn = pair->Key;
        auto properties = jsonMap->GetNamedObject(pfn);
        wstring packageFamilyName = properties->GetNamedString("PackageFamilyName")->Data();
        properties->Insert(L"StartUp", JsonValue::CreateNumberValue(static_cast<double>(GetAppStartUpType(packageFamilyName))));
    }

    return ref new ListAppsResponse(ResponseStatus::Success, jsonMap);
}

IResponse^ HandleTpmGetServiceUrl(IRequest^ request)
{
    TRACE(__FUNCTION__);
    unsigned long logicalDeviceId = 0;
    Utils::TryReadRegistryValue(TpmSlotRegistrySubKey, TpmSlotPropertyName, logicalDeviceId);
    TRACEP(L"logicalDeviceId=", logicalDeviceId);

    std::string serviceUrl = Tpm::GetServiceUrl((uint32_t)logicalDeviceId);
    auto serviceUrlW = Utils::MultibyteToWide(serviceUrl.c_str());
    return ref new StringResponse(ResponseStatus::Success, ref new Platform::String(serviceUrlW.c_str()), request->Tag);
}

IResponse^ HandleTpmGetSASToken(IRequest^ request)
{
    TRACE(__FUNCTION__);
    unsigned long logicalDeviceId = 0;
    Utils::TryReadRegistryValue(TpmSlotRegistrySubKey, TpmSlotPropertyName, logicalDeviceId);
    TRACEP(L"logicalDeviceId=", logicalDeviceId);
    
    std::string sasToken = Tpm::GetSASToken(logicalDeviceId);
    auto sasTokenW = Utils::MultibyteToWide(sasToken.c_str());
    return ref new StringResponse(ResponseStatus::Success, ref new Platform::String(sasTokenW.c_str()), request->Tag);
}

IResponse^ HandleGetWindowsUpdatePolicy(IRequest^ request)
{
    TRACE(__FUNCTION__);

    return WindowsUpdatePolicyCSP::Get(request);
}

IResponse^ HandleSetWindowsUpdatePolicy(IRequest^ request)
{
    TRACE(__FUNCTION__);

    return WindowsUpdatePolicyCSP::Set(request);
}

IResponse^ HandleGetWindowsUpdateRebootPolicy(IRequest^ request)
{
    TRACE(__FUNCTION__);

    auto configuration = ref new WindowsUpdateRebootPolicyConfiguration();

    wstring value;
    if (ERROR_SUCCESS == Utils::TryReadRegistryValue(IoTDMRegistryRoot, RegWindowsUpdateRebootAllowed, value))
    {
        configuration->allow = value == RegTrue;
    }
    else
    {
        // default is to allow rebooting.
        configuration->allow = true;
    }

    return ref new GetWindowsUpdateRebootPolicyResponse(ResponseStatus::Success, configuration);
}

IResponse^ HandleSetWindowsUpdateRebootPolicy(IRequest^ request)
{
    TRACE(__FUNCTION__);

    auto windowsUpdateRebootPolicy = dynamic_cast<SetWindowsUpdateRebootPolicyRequest^>(request);
    assert(windowsUpdateRebootPolicy != nullptr);

    unsigned long returnCode;
    string output;
    wstring command = Utils::GetSystemRootFolder() + L"\\ApplyUpdate.exe ";
    command += windowsUpdateRebootPolicy->configuration->allow ? L"-blockrebootoff" : L"-blockrebooton";
    Utils::LaunchProcess(command, returnCode, output);
    if (returnCode != 0)
    {
        throw DMExceptionWithErrorCode("Error: ApplyUpdate.exe returned an error code.", returnCode);
    }
    Utils::WriteRegistryValue(IoTDMRegistryRoot, RegWindowsUpdateRebootAllowed,
        windowsUpdateRebootPolicy->configuration->allow ? RegTrue : RegFalse);

    return ref new StatusCodeResponse(ResponseStatus::Success, request->Tag);
}

IResponse^ HandleGetWindowsUpdates(IRequest^ request)
{
    TRACE(__FUNCTION__);

    wstring installed = L"<error>";
    wstring approved = L"<error>";
    wstring failed = L"<error>";
    wstring installable = L"<error>";
    wstring pendingReboot = L"<error>";
    wstring lastScanTime = L"<error>";
    bool deferUpgrade;

    MdmProvision::TryGetString(L"./Device/Vendor/MSFT/Update/InstalledUpdates", installed);
    MdmProvision::TryGetString(L"./Device/Vendor/MSFT/Update/ApprovedUpdates", approved);
    MdmProvision::TryGetString(L"./Device/Vendor/MSFT/Update/FailedUpdates", failed);
    MdmProvision::TryGetString(L"./Device/Vendor/MSFT/Update/InstallableUpdates", installable);
    MdmProvision::TryGetString(L"./Device/Vendor/MSFT/Update/PendingRebootUpdates", pendingReboot);
    MdmProvision::TryGetString(L"./Device/Vendor/MSFT/Update/LastSuccessfulScanTime", lastScanTime);
    MdmProvision::TryGetBool(L"./Device/Vendor/MSFT/Update/DeferUpgrade", deferUpgrade);

    auto configuration = ref new GetWindowsUpdatesConfiguration();
    configuration->installed = ref new String(installed.c_str());
    configuration->approved = ref new String(approved.c_str());
    configuration->failed = ref new String(failed.c_str());
    configuration->installable = ref new String(installable.c_str());
    configuration->pendingReboot = ref new String(pendingReboot.c_str());
    configuration->lastScanTime = ref new String(lastScanTime.c_str());
    configuration->deferUpgrade = deferUpgrade;

    return ref new GetWindowsUpdatesResponse(ResponseStatus::Success, configuration);
}

IResponse^ HandleSetWindowsUpdates(IRequest^ request)
{
    TRACE(__FUNCTION__);

    auto windowsUpdatesRequest = dynamic_cast<SetWindowsUpdatesRequest^>(request);
    assert(windowsUpdatesRequest != nullptr);

    MdmProvision::RunAdd(L"./Device/Vendor/MSFT/Update/ApprovedUpdates", windowsUpdatesRequest->configuration->approved->Data());

    return ref new StatusCodeResponse(ResponseStatus::Success, request->Tag);
}

IResponse^ HandleDeviceHealthAttestationVerifyHealth(IRequest^ request)
{
    TRACE(__FUNCTION__);

    auto verifyHealthRequest = dynamic_cast<DeviceHealthAttestationVerifyHealthRequest^>(request);
    assert(verifyHealthRequest != nullptr);

    DeviceHealthAttestationCSP::SetHASEndpoint(verifyHealthRequest->HealthAttestationServerEndpoint->Data());
    DeviceHealthAttestationCSP::ExecVerifyHealth();

    auto healthAttestationStatus = DeviceHealthAttestationCSP::GetStatus();
    for (int i = 0; i < 5 && healthAttestationStatus == 1 /*HEALTHATTESTATION_CERT_RETRIEVAL_REQUESTED*/; i++)
    {
        /* HEALTHATTESTATION_CERT_RETRIEVAL_REQUESTED signifies that the call on the node VerifyHealth 
            has been triggered and now the OS is trying to retrieve DHA-EncBlob from DHA-Server.*/
        Sleep(200);
        healthAttestationStatus = DeviceHealthAttestationCSP::GetStatus();
    }

    if (healthAttestationStatus != 3 /*HEALTHATTESTATION_CERT_RETRIEVAL_COMPLETE*/)
    {
        wstringstream ws;
        ws << L"VerifyHealth failed: 0x" << hex << healthAttestationStatus;
        auto errorMessageCStr = ws.str();
        auto errorMessage = ref new String(errorMessageCStr.c_str(), static_cast<unsigned int>(errorMessageCStr.length()));
        return ref new StringResponse(ResponseStatus::Failure, errorMessage, DMMessageKind::ErrorResponse);
    }
    return ref new StatusCodeResponse(ResponseStatus::Success, request->Tag);
}

IResponse^ HandleDeviceHealthAttestationGetReport(IRequest^ request)
{
    TRACE(__FUNCTION__);

    auto certificateRequest = dynamic_cast<DeviceHealthAttestationGetReportRequest^>(request);
    assert(certificateRequest != nullptr);

    DeviceHealthAttestationCSP::SetNonce(certificateRequest->Nonce->Data());
    auto certificateCStr = DeviceHealthAttestationCSP::GetCertificate();
    auto certificate = ref new Platform::String(certificateCStr.c_str(), static_cast<unsigned int>(certificateCStr.length()));
    auto correlationIdCStr = DeviceHealthAttestationCSP::GetCorrelationId();
    auto correlationId = ref new Platform::String(correlationIdCStr.c_str(), static_cast<unsigned int>(correlationIdCStr.length()));

    return ref new DeviceHealthAttestationGetReportResponse(certificate, correlationId);
}

IResponse^ HandleGetEventTracingConfiguration(IRequest^ request)
{
    TRACE(__FUNCTION__);
    return DiagnosticLogCSP::HandleGetEventTracingConfiguration(request);
}

IResponse^ HandleSetEventTracingConfiguration(IRequest^ request)
{
    TRACE(__FUNCTION__);
    return DiagnosticLogCSP::HandleSetEventTracingConfiguration(request);
}

IResponse^ HandleGetDMFolders(IRequest^ request)
{
    return DMStorage::HandleGetDMFolders(request);
}

IResponse^ HandleGetDMFiles(IRequest^ request)
{
    TRACE(__FUNCTION__);
    return DMStorage::HandleGetDMFiles(request);
}

IResponse^ HandleDeleteDMFile(IRequest^ request)
{
    TRACE(__FUNCTION__);
    return DMStorage::HandleDeleteDMFile(request);
}

// Get request and produce a response
IResponse^ ProcessCommand(IRequest^ request)
{
    TRACE(__FUNCTION__);

    switch (request->Tag)
    {
#define MODEL_NODEF(A, B, C, D) case DMMessageKind::##A: { return Handle##A(request); }
#define MODEL_REQDEF(A, B, C, D) MODEL_NODEF(A, B, C, D)
#define MODEL_ALLDEF(A, B, C, D) MODEL_NODEF(A, B, C, D)
#define MODEL_TAGONLY(A, B, C, D)
#include "Models\ModelsInfo.dat"
#undef MODEL_NODEF
#undef MODEL_REQDEF
#undef MODEL_ALLDEF
#undef MODEL_TAGONLY

    default:
        TRACEP(L"Error: ", Utils::ConcatString(L"Unknown command: ", (uint32_t)request->Tag));
        throw DMException("Error: Unknown command");
    }
}

void EnsureErrorsLogged(const function<void()>& func)
{
    TRACE(__FUNCTION__);

    ErrorResponse^ error;

    try
    {
        func();
    }
    catch (const DMExceptionWithErrorCode& e)
    {
        error = CreateErrorResponse(ErrorSubSystem::DeviceManagement, e.ErrorCode(), e.what());
    }
    catch (const exception& e)  // Note that DMException is just 'exception' with some trace statements.
    {
        error = CreateErrorResponse(ErrorSubSystem::DeviceManagement, static_cast<int>(DeviceManagementErrors::GenericError), e.what());
    }
    catch (Platform::Exception^ e)
    {
        error = ref new ErrorResponse(ErrorSubSystem::DeviceManagement, e->HResult, e->Message);
    }
    catch (...)
    {
        error = ref new ErrorResponse(ErrorSubSystem::DeviceManagement, static_cast<int>(DeviceManagementErrors::GenericError), L"Unknown exception!");
    }

    if (error)
    {
        // We can't do anything but log it...
        TRACE(L"Error encountered!");
        TRACEP(L"  Error Code   : ", error->ErrorCode);
        TRACEP(L"  Error Message: ", error->ErrorMessage->Data());
    }
}

void Listen()
{
    TRACE(__FUNCTION__);

    EnsureErrorsLogged([&]()
    {
        SystemConfiguratorProxyStart();
    });
}
