#include "stdafx.h"
#include "..\SharedUtilities\Logger.h"
#include "..\SharedUtilities\DMRequest.h"
#include "..\SharedUtilities\SecurityAttributes.h"
#include "CSPs\MdmProvision.h"
#include "CSPs\CertificateInfo.h"
#include "CSPs\CertificateManagement.h"
#include "CSPs\RebootCSP.h"
#include "CSPs\EnterpriseModernAppManagementCSP.h"
#include "CSPs\CustomDeviceUiCsp.h"
#include "TimeCfg.h"
#include "AppCfg.h"
#include "TpmSupport.h"

#include <fstream>

#include "Models\AllModels.h"

using namespace Microsoft::Devices::Management::Message;
using namespace std;
using namespace Windows::Data::Json;

IResponse^ HandleFactoryReset(IRequest^ request)
{
    TRACE(__FUNCTION__);

    try
    {
        auto resetRequest = dynamic_cast<FactoryResetRequest^>(request);
        TRACEP(L"clearTPM = ", (resetRequest->clearTPM ? L"true" : L"false"));
        TRACEP(L"recoveryPartitionGUID = ", resetRequest->recoveryPartitionGUID->Data());

        // Clear the TPM if requested...
        if (resetRequest->clearTPM)
        {
            ClearTPM();
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
    catch (const DMException& e)
    {
        TRACEP("ERROR DMCommand::HandleFactoryReset: ", e.what());
        return ref new StatusCodeResponse(ResponseStatus::Failure, request->Tag);
    }
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

    try
    {
        auto setTimeInfoRequest = dynamic_cast<SetTimeInfoRequest^>(request);
        TimeCfg::Set(setTimeInfoRequest);
        return ref new StatusCodeResponse(ResponseStatus::Success, request->Tag);
    }
    catch (const DMException& e)
    {
        TRACEP("ERROR DMCommand::HandleSetTimeInfo: ", e.what());
        return ref new StatusCodeResponse(ResponseStatus::Failure, request->Tag);
    }
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

    try
    {
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
    catch (const DMException& e)
    {
        TRACEP("ERROR DMCommand::HandleSetCertificateConfiguration: ", e.what());
        return ref new StatusCodeResponse(ResponseStatus::Failure, request->Tag);
    }
}

IResponse^ HandleGetCertificateDetails(IRequest^ request)
{
    TRACE(__FUNCTION__);

    try
    {
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
    catch (const DMException& e)
    {
        TRACEP("ERROR DMCommand::HandleGetCertificateDetails: ", e.what());
        return ref new StatusCodeResponse(ResponseStatus::Failure, request->Tag);
    }
}

IResponse^ HandleGetRebootInfo(IRequest^ request)
{
    TRACE(__FUNCTION__);

    try
    {
        GetRebootInfoResponse^ response = ref new GetRebootInfoResponse(ResponseStatus::Success);
        response->singleRebootTime = ref new String(RebootCSP::GetSingleScheduleTime().data());
        response->dailyRebootTime = ref new String(RebootCSP::GetDailyScheduleTime().data());
        response->lastBootTime = ref new String(RebootCSP::GetLastRebootTime().data());
        response->lastRebootCmdTime = ref new String(RebootCSP::GetLastRebootCmdTime().data());
        return response;
    }
    catch (const DMException& e)
    {
        TRACEP("ERROR DMCommand::HandleGetRebootInfo: ", e.what());
        return ref new GetRebootInfoResponse(ResponseStatus::Failure);
    }
}

IResponse^ HandleSetRebootInfo(IRequest^ request)
{
    TRACE(__FUNCTION__);

    try
    {
        auto setRebootInfoRequest = dynamic_cast<SetRebootInfoRequest^>(request);
        RebootCSP::SetSingleScheduleTime(setRebootInfoRequest->singleRebootTime->Data());
        RebootCSP::SetDailyScheduleTime(setRebootInfoRequest->dailyRebootTime->Data());
        return ref new StatusCodeResponse(ResponseStatus::Success, request->Tag);
    }
    catch (const DMException& e)
    {
        TRACEP("ERROR DMCommand::HandleSetRebootInfo: ", e.what());
        return ref new GetRebootInfoResponse(ResponseStatus::Failure);
    }
}

IResponse^ HandleGetTimeInfo(IRequest^ request)
{
    return TimeCfg::Get();
}

IResponse^ HandleImmediateReboot(IRequest^ request)
{
    TRACE(__FUNCTION__);

    try
    {
        auto immediateRebootRequest = dynamic_cast<ImmediateRebootRequest^>(request);
        RebootCSP::ExecRebootNow(immediateRebootRequest->lastRebootCmdTime->Data());
        return ref new StatusCodeResponse(ResponseStatus::Success, request->Tag);
    }
    catch (const DMException& e)
    {
        TRACEP("ERROR DMCommand::HandleImmediateReboot: ", e.what());
        return ref new StatusCodeResponse(ResponseStatus::Failure, request->Tag);
    }
}

IResponse^ HandleCheckUpdates(IRequest^ request)
{
    return ref new CheckForUpdatesResponse(ResponseStatus::Success, true);
}

IResponse^ HandleInstallApp(IRequest^ request)
{
    try
    {
        auto appInstall = dynamic_cast<AppInstallRequest^>(request);
        auto info = appInstall->AppInstallInfo;

        std::vector<wstring> deps;
        for each (auto dep in info->Dependencies)
        {
            deps.push_back((wstring)dep->Data());
        }
        auto packageFamilyName = (wstring)info->PackageFamilyName->Data();
        auto appxPath = (wstring)info->AppxPath->Data();

        EnterpriseModernAppManagementCSP::InstallApp(packageFamilyName, appxPath, deps);
        return ref new StatusCodeResponse(ResponseStatus::Success, request->Tag);
    }
    catch (Platform::Exception^ e)
    {
        std::wstring failure(e->Message->Data());
        TRACEP(L"ERROR DMCommand::HandleInstallApp: ", Utils::ConcatString(failure.c_str(), e->HResult));
        return ref new StatusCodeResponse(ResponseStatus::Failure, request->Tag);
    }
}

IResponse^ HandleUninstallApp(IRequest^ request)
{
    try
    {
        auto appUninstall = dynamic_cast<AppUninstallRequest^>(request);
        auto info = appUninstall->AppUninstallInfo;
        auto packageFamilyName = (wstring)info->PackageFamilyName->Data();
        auto storeApp = info->StoreApp;

        EnterpriseModernAppManagementCSP::UninstallApp(packageFamilyName, storeApp);
        return ref new StatusCodeResponse(ResponseStatus::Success, request->Tag);
    }
    catch (Platform::Exception^ e)
    {
        std::wstring failure(e->Message->Data());
        TRACEP(L"ERROR DMCommand::HandleUninstallApp: ", Utils::ConcatString(failure.c_str(), e->HResult));
        return ref new StatusCodeResponse(ResponseStatus::Failure, request->Tag);
    }
}

IResponse^ HandleTransferFile(IRequest^ request)
{
    try
    {
        auto transferRequest = dynamic_cast<AzureFileTransferRequest^>(request);
        auto info = transferRequest->AzureFileTransferInfo;
        auto upload = info->Upload;
        auto localPath = (wstring)info->LocalPath->Data();
        auto appLocalDataPath = (wstring)info->AppLocalDataPath->Data();

        std::ifstream  src((upload) ? localPath : appLocalDataPath, std::ios::binary);
        std::ofstream  dst((!upload) ? localPath : appLocalDataPath, std::ios::binary);
        dst << src.rdbuf();

        return ref new StatusCodeResponse(ResponseStatus::Success, request->Tag);
    }
    catch (Platform::Exception^ e)
    {
        std::wstring failure(e->Message->Data());
        TRACEP(L"ERROR DMCommand::HandleTransferFile: ", Utils::ConcatString(failure.c_str(), e->HResult));
        return ref new StatusCodeResponse(ResponseStatus::Failure, request->Tag);
    }
}

IResponse^ HandleAppLifecycle(IRequest^ request)
{
    try
    {
        auto appLifecycle = dynamic_cast<AppLifecycleRequest^>(request);
        auto info = appLifecycle->AppLifecycleInfo;
        auto appId = (wstring)info->AppId->Data();
        bool start = info->Start;

        if (start) AppCfg::StartApp(appId);
        else AppCfg::StopApp(appId);
        return ref new StatusCodeResponse(ResponseStatus::Success, request->Tag);
    }
    catch (Platform::Exception^ e)
    {
        std::wstring failure(e->Message->Data());
        TRACEP(L"ERROR DMCommand::HandleAppLifecycle: ", Utils::ConcatString(failure.c_str(), e->HResult));
        return ref new StatusCodeResponse(ResponseStatus::Failure, request->Tag);
    }
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
    try
    {
        auto appId = (wstring)info->AppId->Data();
        auto isBackgroundApp = info->IsBackgroundApplication;

        if (add) { CustomDeviceUiCSP::AddAsStartupApp(appId, isBackgroundApp); }
        else { CustomDeviceUiCSP::RemoveBackgroundApplicationAsStartupApp(appId); }
        return ref new StatusCodeResponse(ResponseStatus::Success, tag);
    }
    catch (Platform::Exception^ e)
    {
        std::wstring failure(e->Message->Data());
        TRACEP(L"ERROR DMCommand::HandleRemoveAppForStartup: ", Utils::ConcatString(failure.c_str(), e->HResult));
        return ref new StatusCodeResponse(ResponseStatus::Failure, tag);
    }
}

IResponse^ HandleAddStartupApp(IRequest^ request)
{
    try
    {
        auto startupApp = dynamic_cast<AddStartupAppRequest^>(request);
        auto info = startupApp->StartupAppInfo;
        return HandleAddRemoveAppForStartup(info, request->Tag, true);
    }
    catch (Platform::Exception^ e)
    {
        std::wstring failure(e->Message->Data());
        TRACEP(L"ERROR DMCommand::HandleAddAppForStartup: ", Utils::ConcatString(failure.c_str(), e->HResult));
        return ref new StatusCodeResponse(ResponseStatus::Failure, request->Tag);
    }
}

IResponse^ HandleRemoveStartupApp(IRequest^ request)
{
    try
    {
        auto startupApp = dynamic_cast<RemoveStartupAppRequest^>(request);
        auto info = startupApp->StartupAppInfo;
        return HandleAddRemoveAppForStartup(info, request->Tag, false);
    }
    catch (Platform::Exception^ e)
    {
        std::wstring failure(e->Message->Data());
        TRACEP(L"ERROR DMCommand::HandleRemoveAppForStartup: ", Utils::ConcatString(failure.c_str(), e->HResult));
        return ref new StatusCodeResponse(ResponseStatus::Failure, request->Tag);
    }
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
    auto jsonMap = JsonObject::Parse(ref new Platform::String(json.c_str()));
    return ref new ListAppsResponse(ResponseStatus::Success, jsonMap);
}

IResponse^ HandleTpmGetServiceUrl(IRequest^ request)
{
    TRACE(__FUNCTION__);
    try
    {
        uint32_t logicalDeviceId = dynamic_cast<TpmGetServiceUrlRequest^>(request)->LogicalDeviceId;
        std::string serviceUrl = GetServiceUrl(logicalDeviceId);
        auto serviceUrlW = Utils::MultibyteToWide(serviceUrl.c_str());
        return ref new StringResponse(ResponseStatus::Success, ref new Platform::String(serviceUrlW.c_str()), request->Tag);
    }
    catch (...)
    {
        TRACE(L"HandleTpmGetServiceUrl failed");
        return ref new StringResponse(ResponseStatus::Failure, L"", request->Tag);
    }
}

IResponse^ HandleTpmGetSASToken(IRequest^ request)
{
    TRACE(__FUNCTION__);
    try
    {
        uint32_t logicalDeviceId = dynamic_cast<TpmGetSASTokenRequest^>(request)->LogicalDeviceId;
        TRACEP(L"logicalDeviceId=", logicalDeviceId);
        std::string sasToken = GetSASToken(logicalDeviceId);
        auto sasTokenW = Utils::MultibyteToWide(sasToken.c_str());
        return ref new StringResponse(ResponseStatus::Success, ref new Platform::String(sasTokenW.c_str()), request->Tag);
    }
    catch (...)
    {
        TRACE(L"HandleTpmGetSASToken failed");
        return ref new StringResponse(ResponseStatus::Failure, L"", request->Tag);
    }
}

IResponse^ HandleGetWindowsUpdatePolicy(IRequest^ request)
{
    TRACE(__FUNCTION__);

    unsigned int activeHoursStart = static_cast<unsigned int>(-1);
    unsigned int activeHoursEnd = static_cast<unsigned int>(-1);
    unsigned int allowAutoUpdate = static_cast<unsigned int>(-1);
    unsigned int allowMUUpdateService = static_cast<unsigned int>(-1);
    unsigned int allowNonMicrosoftSignedUpdate = static_cast<unsigned int>(-1);

    unsigned int allowUpdateService = static_cast<unsigned int>(-1);
    unsigned int branchReadinessLevel = static_cast<unsigned int>(-1);
    unsigned int deferFeatureUpdatesPeriod = static_cast<unsigned int>(-1);    // in days
    unsigned int deferQualityUpdatesPeriod = static_cast<unsigned int>(-1);    // in days
    unsigned int excludeWUDrivers = static_cast<unsigned int>(-1);

    unsigned int pauseFeatureUpdates = static_cast<unsigned int>(-1);
    unsigned int pauseQualityUpdates = static_cast<unsigned int>(-1);
    unsigned int requireUpdateApproval = static_cast<unsigned int>(-1);
    unsigned int scheduledInstallDay = static_cast<unsigned int>(-1);
    unsigned int scheduledInstallTime = static_cast<unsigned int>(-1);

    wstring updateServiceUrl = L"<error>";

    MdmProvision::TryGetNumber<unsigned int>(L"./Device/Vendor/MSFT/Policy/Result/Update/ActiveHoursStart", activeHoursStart);
    MdmProvision::TryGetNumber<unsigned int>(L"./Device/Vendor/MSFT/Policy/Result/Update/ActiveHoursEnd", activeHoursEnd);
    MdmProvision::TryGetNumber<unsigned int>(L"./Device/Vendor/MSFT/Policy/Result/Update/AllowAutoUpdate", allowAutoUpdate);
    MdmProvision::TryGetNumber<unsigned int>(L"./Device/Vendor/MSFT/Policy/Result/Update/AllowMUUpdateService", allowMUUpdateService);
    MdmProvision::TryGetNumber<unsigned int>(L"./Device/Vendor/MSFT/Policy/Result/Update/AllowNonMicrosoftSignedUpdate", allowNonMicrosoftSignedUpdate);

    MdmProvision::TryGetNumber<unsigned int>(L"./Device/Vendor/MSFT/Policy/Result/Update/AllowUpdateService", allowUpdateService);
    MdmProvision::TryGetNumber<unsigned int>(L"./Device/Vendor/MSFT/Policy/Result/Update/BranchReadinessLevel", branchReadinessLevel);
    MdmProvision::TryGetNumber<unsigned int>(L"./Device/Vendor/MSFT/Policy/Result/Update/DeferFeatureUpdatesPeriodInDays", deferFeatureUpdatesPeriod);
    MdmProvision::TryGetNumber<unsigned int>(L"./Device/Vendor/MSFT/Policy/Result/Update/DeferQualityUpdatesPeriodInDays", deferQualityUpdatesPeriod);
    // MdmProvision::TryGetNumber<unsigned int>(L"./Device/Vendor/MSFT/Policy/Result/Update/ExcludeWUDrivers", excludeWUDrivers);

    MdmProvision::TryGetNumber<unsigned int>(L"./Device/Vendor/MSFT/Policy/Result/Update/PauseFeatureUpdates", pauseFeatureUpdates);
    MdmProvision::TryGetNumber<unsigned int>(L"./Device/Vendor/MSFT/Policy/Result/Update/PauseQualityUpdates", pauseQualityUpdates);
    MdmProvision::TryGetNumber<unsigned int>(L"./Device/Vendor/MSFT/Policy/Result/Update/RequireUpdateApproval", requireUpdateApproval);
    MdmProvision::TryGetNumber<unsigned int>(L"./Device/Vendor/MSFT/Policy/Result/Update/ScheduledInstallDay", scheduledInstallDay);
    MdmProvision::TryGetNumber<unsigned int>(L"./Device/Vendor/MSFT/Policy/Result/Update/ScheduledInstallTime", scheduledInstallTime);

    MdmProvision::TryGetString(L"./Device/Vendor/MSFT/Policy/Result/Update/UpdateServiceUrl", updateServiceUrl);

    auto configuration = ref new WindowsUpdatePolicyConfiguration();
    configuration->activeHoursStart = activeHoursStart;
    configuration->activeHoursEnd = activeHoursEnd;
    configuration->allowAutoUpdate = allowAutoUpdate;
    configuration->allowMUUpdateService = allowMUUpdateService;
    configuration->allowNonMicrosoftSignedUpdate = allowNonMicrosoftSignedUpdate;

    configuration->allowUpdateService = allowUpdateService;
    configuration->branchReadinessLevel = branchReadinessLevel;
    configuration->deferFeatureUpdatesPeriod = deferFeatureUpdatesPeriod;
    configuration->deferQualityUpdatesPeriod = deferQualityUpdatesPeriod;
    configuration->excludeWUDrivers = excludeWUDrivers;

    configuration->pauseFeatureUpdates = pauseFeatureUpdates;
    configuration->pauseQualityUpdates = pauseQualityUpdates;
    configuration->requireUpdateApproval = requireUpdateApproval;
    configuration->scheduledInstallDay = scheduledInstallDay;
    configuration->scheduledInstallTime = scheduledInstallTime;

    configuration->updateServiceUrl = ref new String(updateServiceUrl.c_str());
    return ref new GetWindowsUpdatePolicyResponse(ResponseStatus::Success, configuration);
}

IResponse^ HandleSetWindowsUpdatePolicy(IRequest^ request)
{
    // ToDo: We need have a consistent policy on whether we:
    // - apply all or nothing.
    // - apply as much as we can and report and error.

    try
    {
        auto updatePolicyRequest = dynamic_cast<SetWindowsUpdatePolicyRequest^>(request);
        assert(updatePolicyRequest->configuration != nullptr);

        MdmProvision::RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/ActiveHoursStart", static_cast<int>(updatePolicyRequest->configuration->activeHoursStart));
        MdmProvision::RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/ActiveHoursEnd", static_cast<int>(updatePolicyRequest->configuration->activeHoursEnd));
        MdmProvision::RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/AllowAutoUpdate", static_cast<int>(updatePolicyRequest->configuration->allowAutoUpdate));
        MdmProvision::RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/AllowMUUpdateService", static_cast<int>(updatePolicyRequest->configuration->allowMUUpdateService));
        MdmProvision::RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/AllowNonMicrosoftSignedUpdate", static_cast<int>(updatePolicyRequest->configuration->allowNonMicrosoftSignedUpdate));
        MdmProvision::RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/AllowUpdateService", static_cast<int>(updatePolicyRequest->configuration->allowUpdateService));
        MdmProvision::RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/BranchReadinessLevel", static_cast<int>(updatePolicyRequest->configuration->branchReadinessLevel));
        MdmProvision::RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/DeferFeatureUpdatesPeriodInDays", static_cast<int>(updatePolicyRequest->configuration->deferFeatureUpdatesPeriod));
        MdmProvision::RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/DeferQualityUpdatesPeriodInDays", static_cast<int>(updatePolicyRequest->configuration->deferQualityUpdatesPeriod));
        MdmProvision::RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/ExcludeWUDrivers", static_cast<int>(updatePolicyRequest->configuration->excludeWUDrivers));
        MdmProvision::RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/PauseFeatureUpdates", static_cast<int>(updatePolicyRequest->configuration->pauseFeatureUpdates));
        MdmProvision::RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/PauseQualityUpdates", static_cast<int>(updatePolicyRequest->configuration->pauseQualityUpdates));
        MdmProvision::RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/RequireUpdateApproval", static_cast<int>(updatePolicyRequest->configuration->requireUpdateApproval));
        MdmProvision::RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/ScheduledInstallDay", static_cast<int>(updatePolicyRequest->configuration->scheduledInstallDay));
        MdmProvision::RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/ScheduledInstallTime", static_cast<int>(updatePolicyRequest->configuration->scheduledInstallTime));
        MdmProvision::RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/UpdateServiceUrl", wstring(updatePolicyRequest->configuration->updateServiceUrl->Data()));

        return ref new StatusCodeResponse(ResponseStatus::Success, request->Tag);
    }
    catch (const DMException& e)
    {
        TRACEP("ERROR DMCommand::HandleSetWindowsUpdatePolicy: ", e.what());
        return ref new StatusCodeResponse(ResponseStatus::Failure, request->Tag);
    }
}

IResponse^ HandleGetWindowsUpdateRebootPolicy(IRequest^ request)
{
    TRACE(__FUNCTION__);

    try
    {
        auto configuration = ref new WindowsUpdateRebootPolicyConfiguration();

        wstring value;
        if (ERROR_SUCCESS == Utils::TryReadRegistryValue(IoTDMRegistryRoot, IoTDMRegistryWindowsUpdateRebootAllowed, value))
        {
            configuration->allow = value == IoTDMRegistryTrue;
        }
        else
        {
            // default is to allow rebooting.
            configuration->allow = true;
        }

        return ref new GetWindowsUpdateRebootPolicyResponse(ResponseStatus::Success, configuration);
    }
    catch (const DMException& e)
    {
        TRACEP("ERROR DMCommand::HandleGetWindowsUpdateRebootPolicy: ", e.what());
        return ref new GetWindowsUpdateRebootPolicyResponse(ResponseStatus::Failure, ref new WindowsUpdateRebootPolicyConfiguration());
    }
}

IResponse^ HandleSetWindowsUpdateRebootPolicy(IRequest^ request)
{
    TRACE(__FUNCTION__);

    try
    {
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
        Utils::WriteRegistryValue(IoTDMRegistryRoot, IoTDMRegistryWindowsUpdateRebootAllowed,
            windowsUpdateRebootPolicy->configuration->allow ? IoTDMRegistryTrue : IoTDMRegistryFalse);

        return ref new StatusCodeResponse(ResponseStatus::Success, request->Tag);
    }
    catch (const DMException& e)
    {
        TRACEP("ERROR DMCommand::HandleSetWindowsUpdateRebootPolicy: ", e.what());
        return ref new StatusCodeResponse(ResponseStatus::Failure, request->Tag);
    }
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
    try
    {
        auto windowsUpdatesRequest = dynamic_cast<SetWindowsUpdatesRequest^>(request);
        assert(windowsUpdatesRequest != nullptr);

        MdmProvision::RunAdd(L"./Device/Vendor/MSFT/Update/ApprovedUpdates", windowsUpdatesRequest->configuration->approved->Data());

        return ref new StatusCodeResponse(ResponseStatus::Success, request->Tag);
    }
    catch (const DMException& e)
    {
        TRACEP("ERROR DMCommand::HandleSetWindowsUpdates: ", e.what());
        return ref new StatusCodeResponse(ResponseStatus::Failure, request->Tag);
    }
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

class PipeConnection
{
public:

    PipeConnection() :
        _pipeHandle(NULL)
    {}

    void Connect(HANDLE pipeHandle)
    {
        TRACE("Connecting to pipe...");
        if (pipeHandle == NULL || pipeHandle == INVALID_HANDLE_VALUE)
        {
            throw DMException("Error: Cannot connect using an invalid pipe handle.");
        }
        if (!ConnectNamedPipe(pipeHandle, NULL))
        {
            throw DMExceptionWithErrorCode("ConnectNamedPipe Error", GetLastError());
        }
        _pipeHandle = pipeHandle;
    }

    ~PipeConnection()
    {
        if (_pipeHandle != NULL)
        {
            TRACE("Disconnecting from pipe...");
            DisconnectNamedPipe(_pipeHandle);
        }
    }
private:
    HANDLE _pipeHandle;
};

void Listen()
{
    TRACE(__FUNCTION__);

    SecurityAttributes sa(GENERIC_WRITE | GENERIC_READ);

    TRACE("Creating pipe...");
    Utils::AutoCloseHandle pipeHandle = CreateNamedPipeW(
        PipeName,
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        PIPE_UNLIMITED_INSTANCES,
        PipeBufferSize,
        PipeBufferSize,
        NMPWAIT_USE_DEFAULT_WAIT,
        sa.GetSA());

    if (pipeHandle.Get() == INVALID_HANDLE_VALUE)
    {
        throw DMExceptionWithErrorCode("CreateNamedPipe Error", GetLastError());
    }

    while (true)
    {
        PipeConnection pipeConnection;
        TRACE("Waiting for a client to connect...");
        pipeConnection.Connect(pipeHandle.Get());
        TRACE("Client connected...");

        auto request = Blob::ReadFromNativeHandle(pipeHandle.Get64());
        TRACE("Request received...");
        TRACEP(L"    ", Utils::ConcatString(L"request tag:", (uint32_t)request->Tag));
        TRACEP(L"    ", Utils::ConcatString(L"request version:", request->Version));

        try
        {
            IResponse^ response = ProcessCommand(request->MakeIRequest());
            response->Serialize()->WriteToNativeHandle(pipeHandle.Get64());
        }
        catch (const DMException& ex)
        {
            TRACE("DMExeption was thrown from ProcessCommand()...");
            auto response = ref new StringResponse(ResponseStatus::Failure, ref new String(std::wstring(Utils::MultibyteToWide(ex.what())).c_str()), DMMessageKind::ErrorResponse);
            response->Serialize()->WriteToNativeHandle(pipeHandle.Get64());
        }

        // ToDo: How do we exit this loop gracefully?
    }
}