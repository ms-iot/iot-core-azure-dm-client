#pragma once

#include <cstdint>
#include <string>

const int PipeBufferSize = 4096;
const int DataSizeInBytes = 128;
const wchar_t* PipeName = L"\\\\.\\pipe\\dm-client-pipe";

enum class DMCommand
{
    Unknown = 0,
    SystemReset = 1,
    CheckUpdates = 2,

    // Reboot
    RebootSystem = 10,
    SetSingleRebootTime = 11,
    GetSingleRebootTime = 12,
    SetDailyRebootTime = 13,
    GetDailyRebootTime = 14,
    GetLastRebootCmdTime = 15,
    GetLastRebootTime = 16,
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
    char  data[128];

    DMResponse() :
        status(DMStatus::Failed)
    {
        memset(&data, 0, sizeof(data));
    }

    void SetData(const std::wstring& msgw)
    {
        memset(data, 0, sizeof(data));
        size_t bytesToCopy = min(msgw.length() * sizeof(msgw[0]), DataSizeInBytes - 1);
        memcpy(data, msgw.c_str(), bytesToCopy);
        TRACEP(L"Setting response to: ", data);
    }

    void SetData(const wchar_t* msg, DWORD param)
    {
        std::basic_ostringstream<wchar_t> messageStream;
        messageStream << msg << param;

        SetData(messageStream.str());
    }
};
#pragma pack(pop)
