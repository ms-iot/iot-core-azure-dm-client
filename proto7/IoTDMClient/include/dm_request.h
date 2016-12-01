#pragma once

#include <cstdint>

enum class dm_command
{
    unknown = 0,
    system_reset = 1,
    check_updates = 2,
    command_x,
    command_y,
    command_z
};

#pragma pack(push)
#pragma pack(1)
struct dm_request
{
    dm_command command;
    char       data[64];
};

struct dm_response
{
    uint32_t status;
    char     data[64];
    wchar_t  message[256];
};
#pragma pack(pop)
