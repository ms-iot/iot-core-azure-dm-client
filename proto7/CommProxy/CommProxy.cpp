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
    DWORD readByteCount = 0;
    if (WriteFile(pipeHandle.Get(), &request, sizeof(request), &writtenByteCount, NULL))
    {
        TRACE("Reading response from pipe...");
        if (!ReadFile(pipeHandle.Get(), &response, sizeof(response), &readByteCount, NULL))
        {
            TRACE("Error: failed to read from pipe (response)...");
            response.status = DMStatus::Failed;
            response.SetData(L"ReadFile failed, GetLastError=", GetLastError());
        }
        else 
        {
            TRACEP(L" response from pipe...", (UINT)response.status);
            if (response.dataSize)
            {
                char *data = (char*)GlobalAlloc(GMEM_FIXED, response.dataSize);
                if (!ReadFile(pipeHandle.Get(), data, response.dataSize, &readByteCount, NULL))
                {
                    TRACE("Error: failed to read from pipe (response data)...");
                    response.status = DMStatus::Failed;
                    response.SetData(L"ReadFile failed, GetLastError=", GetLastError());
                }
                else
                {
                    response.SetData(data, response.dataSize);
                    TRACEP(L" response data from pipe...", (UINT)response.dataSize);
                }
                GlobalFree(data);
            }
        }
    }
    else
    {
        TRACE("Error: failed to write to pipe...");
        response.status = DMStatus::Failed;
        response.SetData(L"WriteFile failed, GetLastError=", GetLastError());
    }

    return readByteCount;
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

    TRACEP("Writing response to stdout ", sizeof(DMResponse));
    DWORD byteWrittenCount = 0;
    bSuccess = WriteFile(stdoutHandle.Get(), &response, sizeof(DMResponse), &byteWrittenCount, NULL);
    if (!bSuccess || byteWrittenCount != sizeof(DMResponse))
    {
        auto errorCode = GetLastError();
        TRACEP("Error writing response ", errorCode);
        return -1;
    }
    TRACEP("Response sent to stdout ", byteWrittenCount);
    if (response.dataSize)
    {
        byteWrittenCount = 0;
        TRACEP("Write response.data (stdout) ", response.dataSize);
        bSuccess = WriteFile(stdoutHandle.Get(), response.data, response.dataSize, &byteWrittenCount, NULL);
        if (!bSuccess || byteWrittenCount != response.dataSize)
        {
            auto errorCode = GetLastError();
            TRACEP("Error writing response.data ", errorCode);
            return -1;
        }
        TRACEP("Response.data sent to stdout ", byteWrittenCount);
    }

    TRACE("Exiting...");
    return 0;
}