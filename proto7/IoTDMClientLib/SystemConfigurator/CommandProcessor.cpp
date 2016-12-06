#include "stdafx.h"
#include "..\include\dm_request.h"
#include "Utilities\SecurityAttributes.h"

#define BUFSIZE 4096

dm_response process_command(const dm_request& request)
{
    static int cmd_index = 0;
    dm_response response;
    response.status = 0;
    memset(&response.data, 0, sizeof(response.data));
    wcscpy_s(response.message, L"Hello from System Configurator");

    switch (request.command)
    {
    case dm_command::system_reset:
        printf("[%d] Handling `system reset`...\n", cmd_index);
        break;
    case dm_command::check_updates:
        printf("[%d] Handling `check updates`...\n", cmd_index);

        // Checking for updates...
        Sleep(1000);
        // Done!

        response.status = 1;
        break;
    default:
        printf("[%d] Handling unknown command...\n", cmd_index);
        break;
    }

    cmd_index++;

    return response;
}

void listen()
{
    HANDLE hPipe;

    security_attributes sa(GENERIC_WRITE | GENERIC_READ);

    const wchar_t* pipename = L"\\\\.\\pipe\\dm-client-pipe";

    hPipe = CreateNamedPipeW(
        pipename,
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        PIPE_UNLIMITED_INSTANCES,
        BUFSIZE,
        BUFSIZE,
        NMPWAIT_USE_DEFAULT_WAIT,
        sa.get_sa());

    if (hPipe != INVALID_HANDLE_VALUE)
    {
        while (true)
        {
            printf("Waiting for someone to connect...\n");
            if (ConnectNamedPipe(hPipe, NULL) != FALSE)
            {
                dm_request request;
                DWORD dwRead;
                //while (ReadFile(hPipe, buffer, sizeof(buffer) - 1, &dwRead, NULL) != FALSE)
                BOOL readResult = ReadFile(hPipe, &request, sizeof(request), &dwRead, NULL);
                if (readResult && dwRead == sizeof(request))
                {
                    dm_response response = process_command(request);

                    DWORD dwWritten;
                    BOOL writeResult = WriteFile(hPipe,
                        &response,
                        sizeof(response),
                        &dwWritten,
                        NULL);

                    if (!writeResult) {
                        printf("WriteFile Error %d\n", GetLastError());
                        throw sysconfig_exception();
                    }

                }
                else {
                    printf("ReadFile Error %d\n", GetLastError());
                    throw sysconfig_exception();
                }
            }
            else
            {
                printf("ConnectNamedPipe Error %d\n", GetLastError());
                throw sysconfig_exception();
            }
            DisconnectNamedPipe(hPipe);
        }
    }
    else
    {
        printf("CreateNamedPipe Error %d\n", GetLastError());
        throw sysconfig_exception();
    }
}