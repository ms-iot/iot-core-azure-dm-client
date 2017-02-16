#include "stdafx.h"
#include <Windows.h>
#include "..\SharedUtilities\Utils.h"
#include "..\SharedUtilities\Logger.h"
#include "..\SharedUtilities\DMRequest.h"

#include "Blob.h"
#include "StringResponse.h"

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
            throw ref new Exception(HRESULT_FROM_WIN32(GetLastError()), "Cannot open pipe. Make sure SystemConfigurator is running");
        }

        // All pipe instances are busy, so wait for a maximum of 1 second
        // or until an instance becomes available.
        WaitNamedPipe(PipeName, 1000);
    }

    if (pipeHandle.Get() == INVALID_HANDLE_VALUE || pipeHandle.Get() == NULL)
    {
        throw ref new Exception(E_FAIL, "Failed to connect to SystemConfigurator pipe...");
    }

    TRACE("Connected successfully to pipe...");

    TRACE("Writing request to pipe...");

    request->WriteToNativeHandle(pipeHandle.Get64());

    TRACE("Reading response from pipe...");

    Blob^ response = Blob::ReadFromNativeHandle(pipeHandle.Get64());

    TRACE("Done writing and reading.");

    return response;
}

int main(Platform::Array<Platform::String^>^ args)
{
    TRACE(__FUNCTION__);
    Utils::AutoCloseHandle stdinHandle(GetStdHandle(STD_INPUT_HANDLE));
    Utils::AutoCloseHandle stdoutHandle(GetStdHandle(STD_OUTPUT_HANDLE));
    try
    {
        TRACE("Reading request from stdin...");

        Blob^ request = Blob::ReadFromNativeHandle(stdinHandle.Get64());

        try
        {
            request->ValidateVersion();

            Blob^ response = GetResponseFromSystemConfigurator(request, PipeName);

            response->WriteToNativeHandle(stdoutHandle.Get64());
        }
        catch (Exception^ ex)
        {
            auto response = ref new StringResponse(ResponseStatus::Failure, ex->Message, DMMessageKind::ErrorResponse);
            response->Serialize()->WriteToNativeHandle(stdoutHandle.Get64());
        }
        // Return code 0 means the caller should get the output from the output stream
        return 0;
    }
    catch (...)
    {
        // Return code 1 means we could not read data from the input pipe. We did not even try to launch SystemConfigurator
        return 1;
    }
}

