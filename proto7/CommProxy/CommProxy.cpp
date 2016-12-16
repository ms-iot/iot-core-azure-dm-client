#include "stdafx.h"
#include <Windows.h>
#include "..\SharedUtilities\Utils.h"
#include "..\SharedUtilities\Logger.h"
#include "..\SharedUtilities\DMRequest.h"

using namespace std;

// Returns the response size in bytes.
uint32_t SendRequestToSystemConfigurator(const DMRequest& request, DMResponse & response)
{
    TRACE(__FUNCTION__);

    Utils::AutoCloseHandle pipeHandle;
    DWORD writtenByteCount = 0;
    int waitAttemptsLeft = 10;

    while (waitAttemptsLeft--)
    {
        TRACE("Attempting to connect to system configurator pipe...");

        pipeHandle = CreateFileW(PipeName,
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            0,
            NULL);

        // Break if the pipe handle is valid.
        if (pipeHandle.Get() != INVALID_HANDLE_VALUE)
        {
            break;
        }

        // Exit if an error other than ERROR_PIPE_BUSY occurs
        if (GetLastError() != ERROR_PIPE_BUSY)
        {
            response.status = DMStatus::Failed;
            response.SetData(L"CreateFileW failed, GetLastError=", GetLastError());
            return 0;
        }

        // All pipe instances are busy, so wait for a maximum of 1 second
        // or until an instance becomes available.
        WaitNamedPipe(PipeName, 1000);
    }

    if (pipeHandle.Get() == INVALID_HANDLE_VALUE || pipeHandle.Get() == NULL)
    {
        TRACE("Failed to connect to system configurator pipe...");
        response.status = DMStatus::Failed;
        response.SetData(L"Failed to connect to system configurator pipe. GetLastError=", GetLastError());
        return 0;
    }
    TRACE("Connected successfully to pipe...");

    TRACE("Writing request to pipe...");
    if (WriteFile(pipeHandle.Get(), &request, sizeof(request), &writtenByteCount, NULL))
    {
        if (DMResponse::Deserialize(pipeHandle.Get(), response))
        {
            TRACE("Writing request to pipe...");
            return 1;
        }
    }
    else
    {
        TRACE("Error: failed to write to pipe...");
        response.status = DMStatus::Failed;
        response.SetData(L"WriteFile failed, GetLastError=", GetLastError());
    }

    return 0;
}

int main(Platform::Array<Platform::String^>^ args)
{
    TRACE(__FUNCTION__);

    Utils::AutoCloseHandle stdinHandle(GetStdHandle(STD_INPUT_HANDLE));
    Utils::AutoCloseHandle stdoutHandle(GetStdHandle(STD_OUTPUT_HANDLE));

    TRACE("Reading request from stdin...");
    DMRequest request;
    DWORD stdinBytesReadCount = 0;
    BOOL bSuccess = ReadFile(stdinHandle.Get(), &request, sizeof(DMRequest), &stdinBytesReadCount, NULL);
    if (!bSuccess || stdinBytesReadCount != sizeof(DMRequest))
    {
        TRACE("Error: failed to read stdin...");
        return -1;
    }

    TRACE("Processing request...");
    DMResponse response;
    if (0 == SendRequestToSystemConfigurator(request, response))
    {
        TRACE("Error: failed to process request...");
        // Do not return. Let the response propagate to the caller.
    }

    if (!DMResponse::Serialize(stdoutHandle.Get(), response))
    {
        return -1;
    }

    TRACE("Exiting...");
    return 0;
}