#include "stdafx.h"
#include <Windows.h>
#include "..\SharedUtilities\Utils.h"
#include "..\SharedUtilities\Logger.h"
#include "..\SharedUtilities\DMRequest.h"

#include "..\..\src\DMMessage\Blob.h"

using namespace Microsoft::Devices::Management::Message;

using namespace std;

Blob^ GetResponseFromSystemConfigurator(Blob^ request, const wchar_t* pipeName)
{
    TRACE(__FUNCTION__);

    Utils::AutoCloseHandle pipeHandle;
    int waitAttemptsLeft = 10;

    while (waitAttemptsLeft--)
    {
        TRACE("Attempting to connect to system configurator pipe...");

        pipeHandle = CreateFileW(pipeName,
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
            throw ref new Exception(E_FAIL, "Cannot open pipe");
        }

        // All pipe instances are busy, so wait for a maximum of 1 second
        // or until an instance becomes available.
        WaitNamedPipe(PipeName, 1000);
    }

    if (pipeHandle.Get() == INVALID_HANDLE_VALUE || pipeHandle.Get() == NULL)
    {
        throw ref new Exception(E_FAIL, "Failed to connect to system configurator pipe...");
    }

    TRACE("Connected successfully to pipe...");

    TRACE("Writing request to pipe...");

    request->WriteToNativeHandle(pipeHandle.Get());

    TRACE("Reading response from pipe...");

    Blob^ response = Blob::ReadFromNativeHandle(pipeHandle.Get());

    TRACE("Done writing and reading.");

    return response;
}

int main(Platform::Array<Platform::String^>^ args)
{
    TRACE(__FUNCTION__);
    try
    {
        Utils::AutoCloseHandle stdinHandle(GetStdHandle(STD_INPUT_HANDLE));
        Utils::AutoCloseHandle stdoutHandle(GetStdHandle(STD_OUTPUT_HANDLE));

        TRACE("Reading request from stdin...");

        Blob^ request = Blob::ReadFromNativeHandle(stdinHandle.Get());

        request->ValidateVersion();

        Blob^ response = GetResponseFromSystemConfigurator(request, PipeName);

        response->WriteToNativeHandle(stdoutHandle.Get());

        return 0;
    }
    catch (...)
    {
        // TODO: figure out how to traffic the exception to the client, instead of just the error code
        return -1;
    }
}

