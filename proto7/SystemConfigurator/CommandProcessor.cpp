#include "stdafx.h"
#include <windows.h>
#include "..\SharedUtilities\Logger.h"
#include "..\SharedUtilities\DMRequest.h"
#include "..\SharedUtilities\SecurityAttributes.h"
#include "CSPs\RebootCSP.h"
#include "CSPs\EnterpriseModernAppManagementCSP.h"
#include "CSPs\DeviceStatusCSP.h"
#include "CSPs\RemoteWipeCSP.h"
#include "CSPs\CustomDeviceUiCsp.h"
#include "TimeCfg.h"

using namespace std;
using namespace Windows::Data::Json;

void HandleAddAppForStartup(const wstring& json, DMMessage& response)
{
    //
    // JSON expected should reflect this class:
    //        public class StartupAppInfo
    //        {
    //            public string AppId { get; set; }
    //            public bool IsBackgroundApplication { get; set; }
    //        }
    //
    TRACEP(L"DMCommand::HandleAddAppForStartup json=", json);
    try
    {
        auto jsonObject = JsonObject::Parse(ref new Platform::String(json.c_str()));
        wstring appId = jsonObject->GetNamedString(ref new Platform::String(L"AppId"))->Data();
        bool isBackgroundApp = jsonObject->GetNamedBoolean(ref new Platform::String(L"IsBackgroundApplication"));

        CustomDeviceUiCSP::AddAsStartupApp(appId, isBackgroundApp);
        response.SetContext(DMStatus::Succeeded);
    }
    catch (Platform::Exception^ e)
    {
        std::wstring failure(e->Message->Data());
        response.SetData(failure.c_str(), e->HResult);
        response.SetContext(DMStatus::Failed);
    }
}

void HandleRemoveAppForStartup(const wstring& json, DMMessage& response)
{
    //
    // JSON expected should reflect this class:
    //        public class StartupAppInfo
    //        {
    //            public string AppId { get; set; }
    //            public bool IsBackgroundApplication { get; set; }
    //        }
    //
    TRACEP(L"DMCommand::HandleRemoveAppForStartup json=", json);
    try
    {
        auto jsonObject = JsonObject::Parse(ref new Platform::String(json.c_str()));
        wstring appId = jsonObject->GetNamedString(ref new Platform::String(L"AppId"))->Data();

        CustomDeviceUiCSP::RemoveBackgroundApplicationAsStartupApp(appId);
        response.SetContext(DMStatus::Succeeded);
    }
    catch (Platform::Exception^ e)
    {
        std::wstring failure(e->Message->Data());
        response.SetData(failure.c_str(), e->HResult);
        response.SetContext(DMStatus::Failed);
    }
}

void HandleListStartupApps(bool backgroundApps, DMMessage& response)
{
    TRACEP(L"DMCommand::HandleListStartupApps backgroundApps=", backgroundApps);
    wstring json = (backgroundApps) ?
        CustomDeviceUiCSP::GetBackgroundTasksToLaunch() :
        CustomDeviceUiCSP::GetStartupAppId();
    response.SetData(json);
    response.SetContext(DMStatus::Succeeded);
}

void HandleListApps(DMMessage& response)
{
    TRACE(__FUNCTION__);
    wstring json = EnterpriseModernAppManagementCSP::GetInstalledApps();
    //
    // JSON expected should reflect a map<string, AppInfo> of PackageFullName 
    // to AppInfo where AppInfo is:
    //        public class AppInfo
    //        {
    //            public string AppSource{ get; set; }
    //            public string Architecture{ get; set; }
    //            public string InstallDate{ get; set; }
    //            public string InstallLocation{ get; set; }
    //            public string IsBundle{ get; set; }
    //            public string IsFramework{ get; set; }
    //            public string IsProvisioned{ get; set; }
    //            public string Name{ get; set; }
    //            public string PackageFamilyName{ get; set; }
    //            public string PackageStatus{ get; set; }
    //            public string Publisher{ get; set; }
    //            public string RequiresReinstall{ get; set; }
    //            public string ResourceID{ get; set; }
    //            public string Users{ get; set; }
    //            public string Version{ get; set; }
    //        }
    //
    response.SetData(json);
    response.SetContext(DMStatus::Succeeded);
}

void HandleInstallApp(const std::wstring& json, DMMessage& response)
{
    //
    // JSON expected should reflect this class:
    //        public class AppxInstallInfo
    //        {
    //            public string PackageFamilyName{ get; set; }
    //            public string AppxPath{ get; set; }
    //            public List<string> Dependencies{ get; set; }
    //        }
    //
    TRACEP(L"DMCommand::InstallApp json=", json);
    try
    {
        auto jsonObject = JsonObject::Parse(ref new Platform::String(json.c_str()));

        auto dependencyString = ref new Platform::String(L"Dependencies");
        vector<wstring> deps;
        if (jsonObject->HasKey(dependencyString))
        {
            auto dependencyObject = jsonObject->GetNamedArray(dependencyString);
            for (unsigned int i = 0; i < dependencyObject->Size; i++)
            {
                deps.push_back(dependencyObject->GetStringAt(i)->Data());
            }
        }

        wstring packageFamilyName = jsonObject->GetNamedString(ref new Platform::String(L"PackageFamilyName"))->Data();
        wstring appxPath = jsonObject->GetNamedString(ref new Platform::String(L"AppxPath"))->Data();

        EnterpriseModernAppManagementCSP::InstallApp(packageFamilyName, appxPath, deps);
        response.SetContext(DMStatus::Succeeded);
    }
    catch (Platform::Exception^ e)
    {
        std::wstring failure(e->Message->Data());
        response.SetData(Utils::ConcatString(failure.c_str(), e->HResult));
        response.SetContext(DMStatus::Failed);
    }
}

void HandleUninstallApp(const std::wstring& json, DMMessage& response)
{
    //
    // JSON expected should reflect this class:
    //        public class AppxUninstallInfo
    //        {
    //            public string PackageFamilyName{ get; set; }
    //            public bool StoreApp { get; set; }
    //        }
    //
    TRACEP(L"DMCommand::UninstallApp json=", json);
    try
    {
        auto jsonObject = JsonObject::Parse(ref new Platform::String(json.c_str()));
        wstring packageFamilyName = jsonObject->GetNamedString(ref new Platform::String(L"PackageFamilyName"))->Data();
        bool storeApp = jsonObject->GetNamedBoolean(ref new Platform::String(L"StoreApp"));

        EnterpriseModernAppManagementCSP::UninstallApp(packageFamilyName, storeApp);
        response.SetContext(DMStatus::Succeeded);
    }
    catch (Platform::Exception^ e)
    {
        std::wstring failure(e->Message->Data());
        response.SetData(Utils::ConcatString(failure.c_str(), e->HResult));
        response.SetContext(DMStatus::Failed);
    }
}

void ProcessCommand(DMMessage& request, DMMessage& response)
{
    TRACE(__FUNCTION__);
    static int cmdIndex = 0;
    response.SetData(L"Default System Configurator Response.");

    auto command = (DMCommand)request.GetContext();
    auto dataStr = request.GetData();
    auto data = dataStr.c_str();
    switch (command)
    {
    case DMCommand::RebootSystem:
        response.SetData(Utils::ConcatString(L"Handling `reboot system`. cmdIndex = ", cmdIndex));
        response.SetContext(DMStatus::Succeeded);
        RebootCSP::ExecRebootNow();
        break;
    case DMCommand::SetRebootInfo:
        {
            TRACE(L"SetRebootInfo:");
            try
            {
                RebootCSP::SetRebootInfo(request.GetDataW());
                response.SetContext(DMStatus::Succeeded);
            }
            catch (DMException& e)
            {
                response.SetData(e.what(), strlen(e.what()));
                response.SetContext(DMStatus::Failed);
            }
        }
        break;
    case DMCommand::GetRebootInfo:
    {
        try
        {
            TRACE(L"GetRebootInfo:");
            wstring rebootInfoJson = RebootCSP::GetRebootInfoJson();
            TRACEP(L" get json reboot info = ", rebootInfoJson.c_str());
            response.SetData(rebootInfoJson);
            response.SetContext(DMStatus::Succeeded);
        }
        catch (DMException& e)
        {
            response.SetData(e.what(), strlen(e.what()));
            response.SetContext(DMStatus::Failed);
        }
    }
    break;
    case DMCommand::FactoryReset:
        RemoteWipeCSP::DoWipe();
        response.SetData(Utils::ConcatString(L"Handling `factory reset`. cmdIndex = ", cmdIndex));
        response.SetContext(DMStatus::Succeeded);
        break;
    case DMCommand::CheckUpdates:
        response.SetData(Utils::ConcatString(L"Handling `check updates`. cmdIndex = ", cmdIndex));

        // Checking for updates...
        Sleep(1000);
        // Done!

        response.SetContext(DMStatus::Succeeded);
        break;
    case DMCommand::ListApps:
        HandleListApps(response);
        break;
    case DMCommand::InstallApp:
        HandleInstallApp(request.GetDataW(), response);
        break;
    case DMCommand::UninstallApp:
        HandleUninstallApp(request.GetDataW(), response);
        break;
    case DMCommand::GetStartupForegroundApp:
        HandleListStartupApps(false, response);
        break;
    case DMCommand::ListStartupBackgroundApps:
        HandleListStartupApps(true, response);
        break;
    case DMCommand::AddStartupApp:
        HandleAddAppForStartup(request.GetDataW(), response);
        break;
    case DMCommand::RemoveStartupApp:
        HandleRemoveAppForStartup(request.GetDataW(), response);
        break;
    case DMCommand::GetTimeInfo:
        {
            try
            {
                wstring timeInfoJson = TimeCfg::GetTimeInfoJson();
                TRACEP(L" get json time info = ", timeInfoJson.c_str());
                response.SetData(timeInfoJson);
                response.SetContext(DMStatus::Succeeded);
            }
            catch (DMException& e)
            {
                response.SetData(e.what(), strlen(e.what()));
                response.SetContext(DMStatus::Failed);
            }
        }
        break;
    case DMCommand::SetTimeInfo:
        {
            try
            {
                TimeCfg::SetTimeInfo(request.GetDataW());
                response.SetContext(DMStatus::Succeeded);
            }
            catch (DMException& e)
            {
                response.SetData(e.what(), strlen(e.what()));
                response.SetContext(DMStatus::Failed);
            }
        }
        break;
    case DMCommand::GetDeviceStatus:
    {
        try
        {
            wstring deviceStatusJson = DeviceStatusCSP::GetDeviceStatusJson();
            response.SetData(deviceStatusJson);
            response.SetContext(DMStatus::Succeeded);
        }
        catch (DMException& e)
        {
            response.SetData(e.what(), strlen(e.what()));
            response.SetContext(DMStatus::Failed);
        }
    }
    break;
    default:
        response.SetData(Utils::ConcatString(L"Cannot handle unknown command...cmdIndex = ", cmdIndex));
        response.SetContext(DMStatus::Failed);
        break;
    }

    cmdIndex++;
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

        DMMessage request(DMCommand::Unknown);
        if (!DMMessage::ReadFromPipe(pipeHandle.Get(), request))
        {
            throw DMExceptionWithErrorCode("ReadFile Error", GetLastError());
        }
        TRACE("Request received...");
        DMMessage response(DMStatus::Failed);
            
        try
        {
            ProcessCommand(request, response);
        }
        catch (const DMException&)
        {
            // response will still contain the error information, so, let it continue
            // and send it back.
            TRACE("DMExeption was thrown from ProcessCommand()...");
        }

        DMMessage::WriteToPipe(pipeHandle.Get(), response);

        // ToDo: How do we exit this loop gracefully?
    }
}