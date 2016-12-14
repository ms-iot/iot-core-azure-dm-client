#pragma once

#include <cstdint>
#include <string>
#include <vector>
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
    uint32_t dataSize;
    std::vector<char> data;

    DMResponse() :
        status(DMStatus::Failed),
        dataSize(0)
    {
        data.resize(dataSize);
    }

    void SetData(const std::wstring& newData)
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> conv1;
        std::string u8str = conv1.to_bytes(newData);
        SetData(u8str.data(), u8str.size());
    }

    void SetData(const char* newData, uint32_t newDataSize)
    {
        data.assign(newData, newData + newDataSize);
        dataSize = newDataSize;
    }

    void SetData(const wchar_t* msg, DWORD param)
    {
        std::basic_ostringstream<wchar_t> messageStream;
        messageStream << msg << param;
        SetData(messageStream.str());
    }

    static bool Serialize(HANDLE pipeHandle, DMResponse& response)
    {
        TRACE("DMResponse.Serialize...");
        DWORD byteWrittenCount = 0;
        if (!WriteFile(pipeHandle, &response.status, sizeof(DMStatus), &byteWrittenCount, NULL))
        {
            auto errorCode = GetLastError();
            TRACEP("Error writing response.status ", errorCode);
            return false;
        }
        if (!WriteFile(pipeHandle, &response.dataSize, sizeof(uint32_t), &byteWrittenCount, NULL))
        {
            auto errorCode = GetLastError();
            TRACEP("Error writing response.dataSize ", errorCode);
            return false;
        }
        TRACEP("Response sent to pipe ", byteWrittenCount);
        if (response.dataSize)
        {
            byteWrittenCount = 0;
            TRACEP("Write response.data to pipe ", response.dataSize);
            if (!WriteFile(pipeHandle, &response.data[0], response.dataSize, &byteWrittenCount, NULL))
            {
                auto errorCode = GetLastError();
                TRACEP("Error writing response.data ", errorCode);
                return false;
            }
            TRACEP("Response.data sent to pipe ", byteWrittenCount);
        }

        return true;
    }

    static bool Deserialize(HANDLE pipeHandle, DMResponse& response)
    {
        TRACE("DMResponse.Deserialize...");
        DWORD readByteCount = 0;
        if (!ReadFile(pipeHandle, &response.status, sizeof(DMStatus), &readByteCount, NULL))
        {
            TRACE("Error: failed to read from pipe (response)...");
            response.status = DMStatus::Failed;
            response.SetData(L"ReadFile failed, GetLastError=", GetLastError());
            return false;
        }

        if (!ReadFile(pipeHandle, &response.dataSize, sizeof(uint32_t), &readByteCount, NULL))
        {
            TRACE("Error: failed to read from pipe (response)...");
            response.status = DMStatus::Failed;
            response.SetData(L"ReadFile failed, GetLastError=", GetLastError());
            return false;
        }

        TRACEP(L" response read from pipe ", (UINT)readByteCount);
        TRACEP(L" response dataSize from pipe ", (UINT)response.dataSize);
        if (response.dataSize)
        {
            readByteCount = 0;
            std::vector<char> data(response.dataSize);
            if (!ReadFile(pipeHandle, &data[0], response.dataSize, &readByteCount, NULL))
            {
                TRACE("Error: failed to read from pipe (response data)...");
                response.status = DMStatus::Failed;
                response.SetData(L"ReadFile failed, GetLastError=", GetLastError());
                return false;
            }
            else
            {
                TRACEP(L" response.data read from pipe ", (UINT)readByteCount);
                response.SetData(&data[0], response.dataSize);
            }
        }

        return true;
    }
};
#pragma pack(pop)
