#include "stdafx.h"
#include <Windows.h>
#include "..\SharedUtilities\Utils.h"
#include "..\SharedUtilities\Logger.h"
#include "..\SharedUtilities\DMRequest.h"

using namespace std;

// Returns the response size in bytes.
uint32_t SendRequestToSystemConfigurator(const DMMessage& request, DMMessage& response)
{
    TRACE(__FUNCTION__);

    Utils::AutoCloseHandle pipeHandle;
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
            auto errorCode = GetLastError();
            response.SetContext(DMStatus::Failed);
            response.SetData(Utils::ConcatString(L"CreateFileW failed, GetLastError=", errorCode));
            return errorCode;
        }

        // All pipe instances are busy, so wait for a maximum of 1 second
        // or until an instance becomes available.
        WaitNamedPipe(PipeName, 1000);
    }

    if (pipeHandle.Get() == INVALID_HANDLE_VALUE || pipeHandle.Get() == NULL)
    {
        TRACE("Failed to connect to system configurator pipe...");
        auto errorCode = GetLastError();
        response.SetContext(DMStatus::Failed);
        response.SetData(Utils::ConcatString(L"Failed to connect to system configurator pipe. GetLastError=", errorCode));
        return errorCode;
    }
    TRACE("Connected successfully to pipe...");

    TRACE("Writing request to pipe...");
    if (!DMMessage::WriteToPipe(pipeHandle.Get(), request))
    {
        TRACE("Error: failed to write to pipe...");
        auto errorCode = GetLastError();
        response.SetContext(DMStatus::Failed);
        response.SetData(Utils::ConcatString(L"WriteFile failed, GetLastError=", errorCode));
        return errorCode;
    }

    TRACE("Reading response from pipe...");
    if (!DMMessage::ReadFromPipe(pipeHandle.Get(), response))
    {
        TRACE("Error: failed to read from pipe...");
        auto errorCode = GetLastError();
        response.SetContext(DMStatus::Failed);
        response.SetData(Utils::ConcatString(L"WriteFile failed, GetLastError=", errorCode));
        return errorCode;
    }

    TRACE("Done writing and reading.");
    return ERROR_SUCCESS;
}

int main(Platform::Array<Platform::String^>^ args)
{
    TRACE(__FUNCTION__);

    Utils::AutoCloseHandle stdinHandle(GetStdHandle(STD_INPUT_HANDLE));
    Utils::AutoCloseHandle stdoutHandle(GetStdHandle(STD_OUTPUT_HANDLE));

    TRACE("Reading request from stdin...");
    DMMessage request(DMCommand::Unknown);
    if (!DMMessage::ReadFromPipe(stdinHandle.Get(), request))
    {
        TRACE("Error: failed to read stdin...");
        return -1;
    }

    TRACE("Processing request...");
    DMMessage response(DMStatus::Failed);
    auto errorCode = SendRequestToSystemConfigurator(request, response);
    if (ERROR_SUCCESS != errorCode)
    {
        TRACE("Error: failed to process request...");
        // Do not return. Let the response propagate to the caller.
    }

    TRACEP("Response: ", response.GetData().c_str());

    if (!DMMessage::WriteToPipe(stdoutHandle.Get(), response))
    {
        return -1;
    }

    TRACE("Exiting...");
    return 0;
}