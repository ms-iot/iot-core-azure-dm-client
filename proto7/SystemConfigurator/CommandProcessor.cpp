#include "stdafx.h"
#include <windows.h>
#include "..\SharedUtilities\Logger.h"
#include "..\SharedUtilities\DMRequest.h"
#include "..\SharedUtilities\SecurityAttributes.h"
#include "CSPs\RebootCSP.h"

using namespace std;

DMResponse ProcessCommand(const DMRequest& request)
{
    TRACE(__FUNCTION__);
    static int cmdIndex = 0;
    DMResponse response;
    response.SetData(L"Default System Configurator Response.");

    switch (request.command)
    {
    case DMCommand::RebootSystem:
        response.SetData(L"Handling `reboot system`. cmdIndex = ", cmdIndex);
        response.status = DMStatus::Succeeded;
        RebootCSP::ExecRebootNow();
        break;
    case DMCommand::SetSingleRebootTime:
        TRACEP("DMCommand::SetSingleRebootTime value = ", request.data);
        RebootCSP::SetSingleScheduleTime(Utils::MultibyteToWide(request.data));
        response.SetData(L"Handling `set reboot single`. cmdIndex = ", cmdIndex);
        response.status = DMStatus::Succeeded;
        break;
    case DMCommand::GetSingleRebootTime:
    {
        TRACE("DMCommand::GetSingleRebootTime");
        wstring valueString = RebootCSP::GetSingleScheduleTime();
        response.SetData(valueString);
        response.status = DMStatus::Succeeded;
    }
        break;
    case DMCommand::SetDailyRebootTime:
        TRACEP("DMCommand::SetDailyRebootTime value = ", request.data);
        RebootCSP::SetDailyScheduleTime(Utils::MultibyteToWide(request.data));
        response.SetData(L"Handling `set reboot daily`. cmdIndex = ", cmdIndex);
        response.status = DMStatus::Succeeded;
        break;
    case DMCommand::GetDailyRebootTime:
    {
        TRACE("DMCommand::GetDailyRebootTime");
        wstring valueString = RebootCSP::GetDailyScheduleTime();
        response.SetData(valueString);
        response.status = DMStatus::Succeeded;
    }
        break;
    case DMCommand::GetLastRebootCmdTime:
    {
        TRACE("DMCommand::GetLastRebootCmdTime");
        wstring valueString = RebootCSP::GetLastRebootCmdTime();
        response.SetData(valueString);
        response.status = DMStatus::Succeeded;
    }
    break;
    case DMCommand::GetLastRebootTime:
    {
        TRACE("DMCommand::GetLastRebootTime");
        wstring valueString = RebootCSP::GetLastRebootTime();
        response.SetData(valueString);
        response.status = DMStatus::Succeeded;
    }
    break;
    case DMCommand::SystemReset:
        response.SetData(L"Handling `system reset`. cmdIndex = ", cmdIndex);
        response.status = DMStatus::Succeeded;
        break;
    case DMCommand::CheckUpdates:
        response.SetData(L"Handling `check updates`. cmdIndex = ", cmdIndex);

        // Checking for updates...
        Sleep(1000);
        // Done!

        response.status = DMStatus::Succeeded;
        break;
    default:
        response.SetData(L"Handling unknown command...cmdIndex = ", cmdIndex);
        response.status = DMStatus::Failed;
        break;
    }

    cmdIndex++;

    return response;
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

        DMRequest request;
        DWORD readBytes = 0;
        BOOL readResult = ReadFile(pipeHandle.Get(), &request, sizeof(request), &readBytes, NULL);
        if (readResult && readBytes == sizeof(request))
        {
            TRACE("Request received...");
            DMResponse response;
            
            try
            {
                response = ProcessCommand(request);
            }
            catch (const DMException&)
            {
                // response will still contain the error information, so, let it continue
                // and send it back.
                TRACE("DMExeption was thrown from ProcessCommand()...");
            }

            TRACE("Sending response...");
            DWORD writtenBytes = 0;
            if (!WriteFile(pipeHandle.Get(), &response, sizeof(response), &writtenBytes, NULL))
            {
                throw DMExceptionWithErrorCode("WriteFile Error", GetLastError());
            }
        }
        else
        {
            throw DMExceptionWithErrorCode("ReadFile Error", GetLastError());
        }

        // ToDo: How do we exit this loop gracefully?
    }
}