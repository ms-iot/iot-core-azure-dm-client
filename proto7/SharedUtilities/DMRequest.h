#pragma once

#include <cstdint>
#include <string>
#include <codecvt>

const int PipeBufferSize = 4096;
const int DataSizeInBytes = 128;
const wchar_t* PipeName = L"\\\\.\\pipe\\dm-client-pipe";

enum class DMCommand
{
    Unknown = 0,
    SystemReset = 1,
    CheckUpdates = 2,
    ListApps = 3,

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
    size_t dataSize;
    char  *data;

    DMResponse() :
        status(DMStatus::Failed),
        data(nullptr),
        dataSize(0)
    {
    }

    void SetData(const std::wstring& newData)
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> conv1;
        std::string u8str = conv1.to_bytes(newData);
        SetData(u8str.data(), u8str.size());
    }

    void SetData(const char* newData, size_t newDataSize)
    {
        if (data)
        {
            GlobalFree(data);
            data = nullptr;
            dataSize = 0;
        }
        data = (char*)GlobalAlloc(GMEM_FIXED, newDataSize);
        memcpy(data, newData, newDataSize);
        dataSize = newDataSize;

        TRACEP("  DMResponse.dataSize= ", dataSize);
        TRACEP("  DMResponse.data= ", data);
    }
    
    void SetData(const wchar_t* msg, DWORD param)
    {
        std::basic_ostringstream<wchar_t> messageStream;
        messageStream << msg << param;
        SetData(messageStream.str());
    }
};
#pragma pack(pop)
