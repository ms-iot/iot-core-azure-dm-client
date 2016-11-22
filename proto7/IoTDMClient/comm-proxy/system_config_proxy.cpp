#include "stdafx.h"
#include "..\include\dm_request.h"

uint32_t send_request_to_system_configurator(const dm_request& request, dm_response & response)
{
    HANDLE hPipe;
    DWORD dwWritten;

    hPipe = CreateFileW(L"\\\\.\\pipe\\dm-client-pipe",
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);
    if (hPipe != INVALID_HANDLE_VALUE)
    {
        WriteFile(hPipe,
            &request,
            sizeof(request),
            &dwWritten,
            NULL);

        printf("written %d bytes\n", dwWritten);

        DWORD dwRead = 0;
        if(ReadFile(hPipe, &response, sizeof(response), &dwRead, NULL) != FALSE)
        {
            printf("received response %d bytes\n", dwRead);
        }

        CloseHandle(hPipe);

        return dwRead;
    }
    else
    {
        printf("Error %d\n", GetLastError());
    }

    return (0);
}
