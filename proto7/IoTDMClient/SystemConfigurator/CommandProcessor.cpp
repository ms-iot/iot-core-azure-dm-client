#include "stdafx.h"
#include "..\include\dm_request.h"

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
        printf("[%d] Handling system reset...\n", cmd_index);
        break;
    default:
        printf("[%d] Handling unknown command...\n", cmd_index);
        break;
    }

    cmd_index++;

    return response;
}
