#pragma once

#include <cstdint>
#include <string>

const int PipeBufferSize = 4096;
const wchar_t* PipeName = L"\\\\.\\pipe\\dm-client-pipe";

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

    DMRequest() :
        command(DMCommand::Unknown)
    {
        memset(&data, 0, sizeof(data));
    }
};

struct DMResponse
{
    DMStatus status;
    char     data[64];
    wchar_t  message[256];

    DMResponse() :
        status(DMStatus::Failed)
    {
        memset(&data, 0, sizeof(data));
        memset(&message, 0, sizeof(message));
    }

    void SetMessage(const std::wstring& msg)
    {
        wcsncpy_s(message, msg.c_str(), _TRUNCATE);
        TRACEP(L"Setting response to: ", message);
    }

    void SetMessage(const wchar_t* msg, DWORD param)
    {
        std::basic_ostringstream<wchar_t> messageStream;
        messageStream << msg << param;

        SetMessage(messageStream.str());
    }
};
#pragma pack(pop)
