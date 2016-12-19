#include "stdafx.h"
#include <windows.h>
#include "..\SharedUtilities\Logger.h"
#include "..\SharedUtilities\DMRequest.h"
#include "..\SharedUtilities\SecurityAttributes.h"
#include "CSPs\RebootCSP.h"
#include "CSPs\EnterpriseModernAppManagementCSP.h"

using namespace std;
using namespace Windows::Data::Json;

void HandleListApps(DMMessage& response)
{
    TRACE("DMCommand::ListApps");
    wstring json = EnterpriseModernAppManagementCSP::GetInstalledApps();
    response.SetData(json);
    response.SetContext(DMStatus::Succeeded);
}

void HandleInstallApp(const std::wstring& json, DMMessage& response)
{
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
        response.SetData(failure.c_str(), e->HResult);
        response.SetContext(DMStatus::Failed);
    }
}

void HandleUninstallApp(const std::wstring& json, DMMessage& response)
{
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
        response.SetData(failure.c_str(), e->HResult);
        response.SetContext(DMStatus::Failed);
    }
}

void ProcessCommand(DMMessage& request, DMMessage& response)
{
    TRACE(__FUNCTION__);
    static int cmdIndex = 0;
    response.SetData(L"Default System Configurator Response.");

    auto command = (DMCommand)request.GetContext();
    auto data = request.GetData();
    switch (command)
    {
    case DMCommand::RebootSystem:
        response.SetData(L"Handling `reboot system`. cmdIndex = ", cmdIndex);
        response.SetContext(DMStatus::Succeeded);
        RebootCSP::ExecRebootNow();
        break;
    case DMCommand::SetSingleRebootTime:
        TRACEP("DMCommand::SetSingleRebootTime value = ", data);
        RebootCSP::SetSingleScheduleTime(Utils::MultibyteToWide(data));
        response.SetData(L"Handling `set reboot single`. cmdIndex = ", cmdIndex);
        response.SetContext(DMStatus::Succeeded);
        break;
    case DMCommand::GetSingleRebootTime:
    {
        TRACE("DMCommand::GetSingleRebootTime");
        wstring valueString = RebootCSP::GetSingleScheduleTime();
        response.SetData(valueString);
        response.SetContext(DMStatus::Succeeded);
    }
    break;
    case DMCommand::SetDailyRebootTime:
        TRACEP("DMCommand::SetDailyRebootTime value = ", data);
        RebootCSP::SetDailyScheduleTime(Utils::MultibyteToWide(data));
        response.SetData(L"Handling `set reboot daily`. cmdIndex = ", cmdIndex);
        response.SetContext(DMStatus::Succeeded);
        break;
    case DMCommand::GetDailyRebootTime:
    {
        TRACE("DMCommand::GetDailyRebootTime");
        wstring valueString = RebootCSP::GetDailyScheduleTime();
        response.SetData(valueString);
        response.SetContext(DMStatus::Succeeded);
    }
    break;
    case DMCommand::GetLastRebootCmdTime:
    {
        TRACE("DMCommand::GetLastRebootCmdTime");
        wstring valueString = RebootCSP::GetLastRebootCmdTime();
        response.SetData(valueString);
        response.SetContext(DMStatus::Succeeded);
    }
    break;
    case DMCommand::GetLastRebootTime:
    {
        TRACE("DMCommand::GetLastRebootTime");
        wstring valueString = RebootCSP::GetLastRebootTime();
        response.SetData(valueString);
        response.SetContext(DMStatus::Succeeded);
    }
    break;
    case DMCommand::SystemReset:
        response.SetData(L"Handling `system reset`. cmdIndex = ", cmdIndex);
        response.SetContext(DMStatus::Succeeded);
        break;
    case DMCommand::CheckUpdates:
        response.SetData(L"Handling `check updates`. cmdIndex = ", cmdIndex);

        // Checking for updates...
        Sleep(1000);
        // Done!

        response.SetContext(DMStatus::Succeeded);
        break;
    case DMCommand::ListApps:
        HandleListApps(response);
        break;
    case DMCommand::InstallApp:
        HandleInstallApp(std::wstring((wchar_t*)&data[0]), response);
        break;
    case DMCommand::UninstallApp:
        HandleUninstallApp(std::wstring((wchar_t*)&data[0]), response);
        break;
    default:
        response.SetData(L"Handling unknown command...cmdIndex = ", cmdIndex);
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