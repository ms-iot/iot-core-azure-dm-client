#pragma once

#include <cstdint>

enum class DMCommand
{
    Unknown = 0,
    SystemReset = 1,
    CheckUpdates = 2,
    CommandX,
    CommandY,
    CommandZ
};

enum class DMStatus
{
    Succeeded = 0,
    Failed = 1,
};

#pragma pack(push)
#pragma pack(1)
struct DMRequest
{
    DMCommand command;
    char      data[64];
};

struct DMResponse
{
    DMStatus status;
    char     data[64];
    wchar_t  message[256];
};
#pragma pack(pop)
