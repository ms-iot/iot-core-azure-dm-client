#include "stdafx.h"
#include <Windows.h>
#include "..\SharedUtilities\Utils.h"
#include "..\SharedUtilities\Logger.h"
#include "..\SharedUtilities\DMRequest.h"

using namespace std;

void SetResponseMessage(DMResponse& response, const wchar_t* errorMessage, DWORD errorCode)
{
    basic_ostringstream<wchar_t> messageStream;
    messageStream << errorMessage << errorCode;

    wstring message = messageStream.str();
    wcsncpy_s(response.message, messageStream.str().c_str(), _TRUNCATE);
}

// Returns the response size in bytes.
uint32_t SendRequestToSystemConfigurator(const DMRequest& request, DMResponse & response)
{
    TRACE(__FUNCTION__);

    Utils::AutoCloseHandle pipeHandle;
    DWORD writtenByteCount = 0;
    int waitAttemptsLeft = 10;

    const wchar_t* namedPipeName = L"\\\\.\\pipe\\dm-client-pipe";

    while (waitAttemptsLeft--)
    {
        TRACE("Attempting to connect to system configurator pipe...");

        pipeHandle = CreateFileW(namedPipeName,
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
            swprintf_s(response.message, L"CreateFileW failed, GetLastError=%d", GetLastError());
            return 0;
        }

        // All pipe instances are busy, so wait for a maximum of 1 second
        // or until an instance becomes available.
        WaitNamedPipe(namedPipeName, 1000);
    }

    if (pipeHandle.Get() == INVALID_HANDLE_VALUE || pipeHandle.Get() == NULL)
    {
        TRACE("Failed to connect to system configurator pipe...");
        response.status = DMStatus::Failed;
        SetResponseMessage(response, L"Failed to connect to system configurator pipe. GetLastError=", GetLastError());
        return 0;
    }
    TRACE("Connected successfully to pipe...");

    TRACE("Writing request to pipe...");
    DWORD readByteCount = 0;
    if (WriteFile(pipeHandle.Get(), &request, sizeof(request), &writtenByteCount, NULL))
    {
        TRACE("Reading response from pipe...");
        if (!ReadFile(pipeHandle.Get(), &response, sizeof(response), &readByteCount, NULL))
        {
            TRACE("Error: failed to read from pipe...");
            response.status = DMStatus::Failed;
            SetResponseMessage(response, L"ReadFile failed, GetLastError=", GetLastError());
        }
    }
    else
    {
        TRACE("Error: failed to write to pipe...");
        response.status = DMStatus::Failed;
        SetResponseMessage(response, L"WriteFile failed, GetLastError=", GetLastError());
    }

    return readByteCount;
}

[Platform::MTAThread]
int wmain(int argc, wchar_t *argv[])
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
        return -1;
    }

    TRACE("Writing response to stdout...");
    DWORD byteWrittenCount = 0;
    bSuccess = WriteFile(stdoutHandle.Get(), &response, sizeof(DMResponse), &byteWrittenCount, NULL);
    if (!bSuccess || byteWrittenCount != sizeof(DMResponse))
    {
        TRACE("Error: failed to write to stdout...");
        return -1;
    }

    TRACE("Exiting...");
    return 0;
}