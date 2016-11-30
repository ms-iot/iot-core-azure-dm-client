#include "stdafx.h"
#include "..\include\dm_request.h"

uint32_t send_request_to_system_configurator(const dm_request& request, dm_response & response)
{
    HANDLE hPipe;
    DWORD dwWritten;
    int wait_attempts_left = 10;

    const wchar_t* namedPipeName = L"\\\\.\\pipe\\dm-client-pipe";

    while (1)
    {
        hPipe = CreateFileW(namedPipeName,
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            0,
            NULL);

        // Break if the pipe handle is valid.
        if (hPipe != INVALID_HANDLE_VALUE)
            break;

        // Exit if an error other than ERROR_PIPE_BUSY occurs
        if (GetLastError() != ERROR_PIPE_BUSY)
        {
            swprintf_s(response.message, L"CreateFileW failed, GetLastError=%d", GetLastError());
            return 0;
        }

        // All pipe instances are busy, so wait for 1 second
        if (!WaitNamedPipe(namedPipeName, 300))
        {
            if (wait_attempts_left-- == 0)
            {
                swprintf_s(response.message, L"Could not open pipe: 20 second wait timed out.");
                return 0;
            }
        }
    }

    if (WriteFile(hPipe, &request, sizeof(request), &dwWritten, NULL))
    {
        DWORD dwRead = 0;
        if (!ReadFile(hPipe, &response, sizeof(response), &dwRead, NULL))
        {
            swprintf_s(response.message, L"ReadFile failed, GetLastError=%d", GetLastError());
        }
        CloseHandle(hPipe);

        return dwRead;
    }
    else
    {
        swprintf_s(response.message, L"WriteFile failed, GetLastError=%d", GetLastError());
    }

    return (0);
}
