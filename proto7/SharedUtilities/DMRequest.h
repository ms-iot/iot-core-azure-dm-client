#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <codecvt>

const int PipeBufferSize = 4096;
const int DataSizeInBytes = 128;
#define PIPE_NAME L"\\\\.\\pipe\\dm-client-pipe"
const wchar_t* PipeName = PIPE_NAME;

enum class DMCommand : uint32_t
{
    Unknown = 0,
    FactoryReset = 1,
    CheckUpdates = 2,
    ListApps = 3,
    InstallApp = 4,
    UninstallApp = 5,

    // Reboot
    RebootSystem = 10,
    SetRebootInfo = 11,
    GetRebootInfo = 12,

    // TimeInfo
    GetTimeInfo = 30,
    SetTimeInfo = 31,

    // Device Status
    GetDeviceStatus = 40,
};

enum class DMStatus : uint32_t
{
    Succeeded = 0,
    Failed = 1,
};

#pragma pack(push)
#pragma pack(1)
class DMMessage
{
private:
    uint32_t _context;
    std::vector<char> _data;

public:
    DMMessage(DMStatus status) : DMMessage((uint32_t)status, 0)
    {
    }
    DMMessage(DMCommand command) : DMMessage((uint32_t)command, 0)
    {
    }

private:
    DMMessage(uint32_t ctxt, uint32_t dataSize) : _context(ctxt)
    {
        _data.resize(dataSize);
    }

public:

    void DumpData()
    {
        TRACE(__FUNCTION__);
        if (_data.size() == 0)
        {
            TRACE("Data size is 0");
        }
        else
        {
            std::basic_ostringstream<char> messageStream0;
            messageStream0 << _data.size();
            TRACEP("Data size is ", messageStream0.str().c_str());

            for (size_t i = 0; i < _data.size() && i < 30; ++i)
            {
                std::basic_ostringstream<char> messageStream1;
                messageStream1 << "_data[" << i << "] ASCII " << (unsigned int)((unsigned char)_data[i]);
                TRACE(messageStream1.str().c_str());
            }
        }
    }

    std::string GetData() const
    {
        return std::string(_data.data(), _data.size());
    }
    std::wstring GetDataW() const
    {
        return std::wstring((wchar_t*)_data.data(), _data.size() / sizeof(wchar_t));
    }

    uint32_t GetDataCount() const
    {
        return (_data.size());
    }

    uint32_t GetContext() const
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

    static bool WriteToPipe(HANDLE pipeHandle, const DMMessage& message)
    {
        TRACE("DMMessage.WriteToPipe...");
        DWORD byteWrittenCount = 0;
        auto context = message.GetContext();
        if (!WriteFile(pipeHandle, &context, sizeof(uint32_t), &byteWrittenCount, NULL) || byteWrittenCount != sizeof(uint32_t))
        {
            // TODO: should this throw a DMException

            TRACEP("Error: failed to write to pipe (context)...", GetLastError());
            return false;
        }
        TRACEP(L" context written to pipe=", context);

        byteWrittenCount = 0;
        auto dataSize = message.GetDataCount();
        if (!WriteFile(pipeHandle, &dataSize, sizeof(uint32_t), &byteWrittenCount, NULL) || byteWrittenCount != sizeof(uint32_t))
        {
            // TODO: should this throw a DMException

            TRACEP("Error: failed to write to pipe (dataSize)...", GetLastError());
            return false;
        }
        TRACEP(L" dataSize written to pipe ", dataSize);

        if (dataSize)
        {
            byteWrittenCount = 0;
            auto data = message.GetData();
            if (!WriteFile(pipeHandle, data.data(), dataSize, &byteWrittenCount, NULL) || byteWrittenCount != dataSize)
            {
                // TODO: should this throw a DMException

                TRACEP("Error: failed to write to pipe (dataSize)...", GetLastError());
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
            // TODO: should this throw a DMException rather than sending a response?

            TRACE("Error: failed to read from pipe (context)...");
            message.SetContext(DMStatus::Failed);
            message.SetData(Utils::ConcatString(L"ReadFile failed, GetLastError=", GetLastError()));
            return false;
        }
        message.SetContext(context);
        TRACEP(L" context read from pipe=", context);

        uint32_t dataSize = 0;
        if (!ReadFile(pipeHandle, &dataSize, sizeof(uint32_t), &readByteCount, NULL) || readByteCount != sizeof(uint32_t))
        {
            // TODO: should this throw a DMException rather than sending a response?

            TRACE("Error: failed to read from pipe (dataSize)...");
            message.SetContext(DMStatus::Failed);
            message.SetData(Utils::ConcatString(L"ReadFile failed, GetLastError=", GetLastError()));
            return false;
        }
        TRACEP(L" dataSize read from pipe=", dataSize);

        if (dataSize)
        {
            readByteCount = 0;
            std::vector<char> data(dataSize, '\0');
            if (!ReadFile(pipeHandle, &data[0], dataSize, &readByteCount, NULL) || readByteCount != dataSize)
            {
                // TODO: should this throw a DMException rather than sending a response?

                TRACE("Error: failed to read from pipe (data)...");
                message.SetContext(DMStatus::Failed);
                message.SetData(Utils::ConcatString(L"ReadFile failed, GetLastError=", GetLastError()));
                return false;
            }
            message.SetData(&data[0], data.size());
        }

        return true;
    }
};
#pragma pack(pop)
