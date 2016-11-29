#include "stdafx.h"
#include "..\include\dm_request.h"

uint32_t send_request_to_system_configurator(const dm_request& request, dm_response & response);

int main()
{
    auto stdinHandle = GetStdHandle(STD_INPUT_HANDLE);
    auto stdoutHandle = GetStdHandle(STD_OUTPUT_HANDLE);

    dm_request request;

    DWORD read_from_stdin;
    BOOL bSuccess = ReadFile(stdinHandle, &request, sizeof(dm_request), &read_from_stdin, NULL);

    if (!bSuccess || read_from_stdin != sizeof(dm_request))
    {
        return -1;
    }
    else
    {
        dm_response response;

        send_request_to_system_configurator(request, response);

        DWORD bytes_written;
        bSuccess = WriteFile(stdoutHandle, &response, sizeof(dm_response), &bytes_written, NULL);

        if (!bSuccess || bytes_written != sizeof(dm_response))
        {
            return -1;
        }
        return 0;
    }
}

