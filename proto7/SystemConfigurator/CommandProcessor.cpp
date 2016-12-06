#include "stdafx.h"
#include <windows.h>
#include "..\SharedUtilities\Logger.h"
#include "..\SharedUtilities\DMRequest.h"
#include "..\SharedUtilities\SecurityAttributes.h"

#define PIPE_BUFFER_SIZE 4096

DMResponse ProcessCommand(const DMRequest& request)
{
    TRACE(__FUNCTION__);
    static int cmdIndex = 0;
    DMResponse response;
    response.status = DMStatus::Failed;
    memset(&response.data, 0, sizeof(response.data));
    wcscpy_s(response.message, L"Hello from System Configurator");

    switch (request.command)
    {
    case DMCommand::SystemReset:
        TRACEP("Handling `system reset`. cmdIndex = ", cmdIndex);
        break;
    case DMCommand::CheckUpdates:
        TRACEP("Handling `check updates`. cmdIndex = ", cmdIndex);

        // Checking for updates...
        Sleep(1000);
        // Done!

        response.status = DMStatus::Succeeded;
        break;
    default:
        TRACEP("Handling unknown command...cmdIndex = ", cmdIndex);
        break;
    }

    cmdIndex++;

    return response;
}

void Listen()
{
    TRACE(__FUNCTION__);

    SecurityAttributes sa(GENERIC_WRITE | GENERIC_READ);

    TRACE("Creating pipe...");
    const wchar_t* pipeName = L"\\\\.\\pipe\\dm-client-pipe";
    HANDLE pipeHandle = CreateNamedPipeW(
        pipeName,
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        PIPE_UNLIMITED_INSTANCES,
        PIPE_BUFFER_SIZE,
        PIPE_BUFFER_SIZE,
        NMPWAIT_USE_DEFAULT_WAIT,
        sa.GetSA());

    if (pipeHandle == INVALID_HANDLE_VALUE)
    {
        throw DMExceptionWithErrorCode("CreateNamedPipe Error", GetLastError());
    }

    while (true)
    {
        TRACE("Waiting for a client to connect...");
        if (ConnectNamedPipe(pipeHandle, NULL) != FALSE)
        {
            TRACE("Client connected...");

            DMRequest request;
            DWORD readBytes = 0;
            BOOL readResult = ReadFile(pipeHandle, &request, sizeof(request), &readBytes, NULL);
            if (readResult && readBytes == sizeof(request))
            {
                TRACE("Request received...");
                DMResponse response = ProcessCommand(request);

                TRACE("Sending response...");
                DWORD writtenBytes = 0;
                if (!WriteFile(pipeHandle, &response, sizeof(response), &writtenBytes, NULL))
                {
                    throw DMExceptionWithErrorCode("WriteFile Error", GetLastError());
                }
            }
            else
            {
                throw DMExceptionWithErrorCode("ReadFile Error", GetLastError());
            }
        }
        else
        {
            throw DMExceptionWithErrorCode("ConnectNamedPipe Error", GetLastError());
        }
        TRACE("Disconnecting pipe...");
        DisconnectNamedPipe(pipeHandle);

        // ToDo: How do we exit?
    }

    CloseHandle(pipeHandle);
}