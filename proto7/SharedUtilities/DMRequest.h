#pragma once

#include <cstdint>
#include <string>

const int PipeBufferSize = 4096;
const wchar_t* PipeName = L"\\\\.\\pipe\\dm-client-pipe";

enum class DMCommand
{
    Unknown = 0,
    SystemReboot = 1,
    SystemReset = 2,
	CheckUpdates = 3,
	ListApps = 4,
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

static const size_t DMRESPONSE_DATA_SIZE = 64;
static const size_t DMRESPONSE_MESSAGE_SIZE = 1024;
struct DMResponse
{
    DMStatus status;
    char     data[DMRESPONSE_DATA_SIZE];
    wchar_t  message[DMRESPONSE_MESSAGE_SIZE];
	size_t    chunkIndex;
	size_t    chunkCount;

    DMResponse() :
        status(DMStatus::Failed),
		chunkIndex(0),
		chunkCount(1)
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
