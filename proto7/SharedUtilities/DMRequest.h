#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <codecvt>

const int PipeBufferSize = 4096;
const int DataSizeInBytes = 128;
const wchar_t* PipeName = L"\\\\.\\pipe\\dm-client-pipe";

enum class DMCommand : uint32_t
{
    Unknown = 0,
    SystemReset = 1,
    CheckUpdates = 2,
    ListApps = 3,
    InstallApp = 4,
    UninstallApp = 5,

    // Reboot
    RebootSystem = 10,
    SetSingleRebootTime = 11,
    GetSingleRebootTime = 12,
    SetDailyRebootTime = 13,
    GetDailyRebootTime = 14,
    GetLastRebootCmdTime = 15,
    GetLastRebootTime = 16,
};

enum class DMStatus : uint32_t
{
    Succeeded = 0,
    Failed = 1,
};

#pragma pack(push)
#pragma pack(1)
struct DMMessage
{
    uint32_t _context;
    std::vector<char> _data;

    DMMessage(DMStatus status) : DMMessage((uint32_t)status, 0)
    {
    }
    DMMessage(DMCommand command) : DMMessage((uint32_t)command, 0)
    {
    }
    DMMessage(uint32_t ctxt, uint32_t dataSize) : _context(ctxt)
    {
        _data.resize(dataSize);
    }

    const char* GetData()
    {
        return (_data.data());
    }
    uint32_t GetDataCount()
    {
        return (_data.size());
    }
    uint32_t GetContext()
    {
        return (_context);
    }

    void SetContext(DMStatus status)
    {
        SetContext((uint32_t)status);
    }

    void SetContext(DMCommand command)
    {
        SetContext((uint32_t)command);
    }

    void SetContext(uint32_t ctxt)
    {
        _context = ctxt;
    }

    void SetData(const wchar_t* msg, DWORD param)
    {
        std::basic_ostringstream<wchar_t> messageStream;
        messageStream << msg << param;
        SetData(messageStream.str());
    }

    void SetData(const std::wstring& newData)
    {
        auto wdataAsBytes = (char*)newData.data();
        auto size = newData.size() * sizeof(wchar_t);
        SetData(wdataAsBytes, size);
    }

    void SetData(const char* newData, uint32_t newDataSize)
    {
        _data.assign(newData, newData + newDataSize);
    }

    static bool WriteToPipe(HANDLE pipeHandle, struct DMMessage& message)
    {
        TRACE("DMMessage.WriteToPipe...");
        DWORD byteWrittenCount = 0;
        auto context = message.GetContext();
        if (!WriteFile(pipeHandle, &context, sizeof(uint32_t), &byteWrittenCount, NULL) || byteWrittenCount != sizeof(uint32_t))
        {
            auto errorCode = GetLastError();
            TRACEP("Error: failed to write to pipe (context)...", errorCode);
            return false;
        }
        TRACEP(L" context written to pipe=", context);

        byteWrittenCount = 0;
        auto dataSize = message.GetDataCount();
        if (!WriteFile(pipeHandle, &dataSize, sizeof(uint32_t), &byteWrittenCount, NULL) || byteWrittenCount != sizeof(uint32_t))
        {
            auto errorCode = GetLastError();
            TRACEP("Error: failed to write to pipe (dataSize)...", errorCode);
            return false;
        }
        TRACEP(L" dataSize written to pipe ", dataSize);

        if (dataSize)
        {
            byteWrittenCount = 0;
            auto data = message.GetData();
            if (!WriteFile(pipeHandle, data, dataSize, &byteWrittenCount, NULL) || byteWrittenCount != dataSize)
            {
                auto errorCode = GetLastError();
                TRACEP("Error: failed to write to pipe (dataSize)...", errorCode);
                return false;
            }
        }

        return true;
    }

    static bool ReadFromPipe(HANDLE pipeHandle, DMMessage& message)
    {
        TRACE("DMMessage.ReadFromPipe...");
        DWORD readByteCount = 0;
        uint32_t context = 0;
        if (!ReadFile(pipeHandle, &context, sizeof(uint32_t), &readByteCount, NULL) || readByteCount != sizeof(uint32_t))
        {
            TRACE("Error: failed to read from pipe (context)...");
            message.SetContext(DMStatus::Failed);
            message.SetData(L"ReadFile failed, GetLastError=", GetLastError());
            return false;
        }
        else
        {
            message.SetContext(context);
        }
        TRACEP(L" context read from pipe=", context);

        uint32_t dataSize = 0;
        if (!ReadFile(pipeHandle, &dataSize, sizeof(uint32_t), &readByteCount, NULL) || readByteCount != sizeof(uint32_t))
        {
            TRACE("Error: failed to read from pipe (dataSize)...");
            message.SetContext(DMStatus::Failed);
            message.SetData(L"ReadFile failed, GetLastError=", GetLastError());
            return false;
        }
        TRACEP(L" dataSize read from pipe=", dataSize);

        if (dataSize)
        {
            readByteCount = 0;
            std::vector<char> data(dataSize + 2, '\0');
            if (!ReadFile(pipeHandle, &data[0], dataSize, &readByteCount, NULL) || readByteCount != dataSize)
            {
                TRACE("Error: failed to read from pipe (data)...");
                message.SetContext(DMStatus::Failed);
                message.SetData(L"ReadFile failed, GetLastError=", GetLastError());
                return false;
            }
            else
            {
                std::wstring wData((wchar_t*)&data[0]);
                wData.resize(dataSize / 2);
                wData.push_back(L'\0');
                message.SetData(wData);
            }
        }

        return true;
    }
};
#pragma pack(pop)
