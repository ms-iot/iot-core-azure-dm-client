#include "stdafx.h"
#include "..\SharedUtilities\Logger.h"
#include "..\SharedUtilities\DMRequest.h"
#include "..\SharedUtilities\SecurityAttributes.h"
#include "CSPs\RebootCSP.h"
#include "CSPs\EnterpriseModernAppManagementCSP.h"
#include "CSPs\DeviceStatusCSP.h"
#include "CSPs\CustomDeviceUiCsp.h"
#include "TimeCfg.h"
#include "AppCfg.h"

#include <fstream>

#include "Models\AllModels.h"

using namespace Microsoft::Devices::Management::Message;
using namespace std;
using namespace Windows::Data::Json;

IResponse^ HandleFactoryReset(IRequest^ request)
{
    return ref new StatusCodeResponse(ResponseStatus::Success, request->Tag);
}

IResponse^ HandleGetDeviceStatus(IRequest^ request)
{
    throw DMExceptionWithErrorCode("Unsupported request: ", (uint32_t)request->Tag);
    //try
    //{
    //    wstring deviceStatusJson = DeviceStatusCSP::GetDeviceStatusJson();
    //    return ref new StatusCodeResponse(ResponseStatus::Success, request->Tag);
    //}
    //catch (DMException& e)
    //{
    //    TRACEP("ERROR DMCommand::HandleGetDeviceStatus: ", e.what());
    //    return ref new StatusCodeResponse(ResponseStatus::Failure, request->Tag);
    //}
}

IResponse^ HandleSetTimeInfo(IRequest^ request)
{
    TRACE(__FUNCTION__);

    try
    {
        TimeCfg::SetTimeInfo(request);
        return ref new StatusCodeResponse(ResponseStatus::Success, request->Tag);
    }
    catch (DMException& e)
    {
        TRACEP("ERROR DMCommand::HandleSetTimeInfo: ", e.what());
        return ref new StatusCodeResponse(ResponseStatus::Failure, request->Tag);
    }
}

IResponse^ HandleGetRebootInfo(IRequest^ request)
{
    throw DMExceptionWithErrorCode("Unsupported request: ", (uint32_t)request->Tag);
    //auto getRebootInfoRequest = dynamic_cast<GetRebootInfoRequest^>(request);
    //auto rebootInfoJson = RebootCSP::GetRebootInfoJson();
    //return ref new StatusCodeResponse(ResponseStatus::Success, request->Tag);
}

IResponse^ HandleSetRebootInfo(IRequest^ request)
{
    throw DMExceptionWithErrorCode("Unsupported request: ", (uint32_t)request->Tag);
    //auto setRebootInfoRequest = dynamic_cast<SetRebootInfoRequest^>(request);
    //wstring json = L"";
    //RebootCSP::SetRebootInfo(json);
    //return ref new StatusCodeResponse(ResponseStatus::Success, request->Tag);
}

IResponse^ HandleGetTimeInfo(IRequest^ request)
{
    return TimeCfg::GetTimeInfo();
}
IResponse^ HandleImmediateReboot(IRequest^ request)
{
    RebootCSP::ExecRebootNow();
    return ref new StatusCodeResponse(ResponseStatus::Success, request->Tag);
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

// Get request and produce a response
IResponse^ ProcessCommand(IRequest^ request)
{
    TRACE(__FUNCTION__);

    switch (request->Tag)
    {
#define MODEL_NODEF(A, B, C, D) case DMMessageKind::##A: { return Handle##A(request); }
#define MODEL_REQDEF(A, B, C, D) MODEL_NODEF(A, B, C, D)
#define MODEL_ALLDEF(A, B, C, D) MODEL_NODEF(A, B, C, D)
#include "Models\ModelsInfo.dat"
#undef MODEL_NODEF
#undef MODEL_REQDEF
#undef MODEL_ALLDEF

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

        auto request = Blob::ReadFromNativeHandle(pipeHandle.Get());
        TRACE("Request received...");
        TRACEP(L"    ", Utils::ConcatString(L"request tag:", (uint32_t)request->Tag));
        TRACEP(L"    ", Utils::ConcatString(L"request version:", request->Version));

        try
        {
            IResponse^ response = ProcessCommand(request->MakeIRequest());
            response->Serialize()->WriteToNativeHandle(pipeHandle.Get());
        }
        catch (const DMException&)
        {
            // TODO: figure out how to respond with an error that can be meaningfully handled.
            //       Is this problem fatal? So we could just die here...
            TRACE("DMExeption was thrown from ProcessCommand()...");
            throw;
        }

        // ToDo: How do we exit this loop gracefully?
    }
}