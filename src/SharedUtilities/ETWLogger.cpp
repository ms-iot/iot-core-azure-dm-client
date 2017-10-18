#include "stdafx.h"
#include <windows.h>
#include <filesystem>
#include <TraceLoggingProvider.h>
#include "ETWLogger.h"

#define PROVIDER_NAME "AzureDM"
// D198EE7D-C5F1-4F5F-95BE-A2EE6FA45897
#define PROVIVDER_GUID (0xD198EE7D, 0xC5F1, 0x4F5F, 0x95, 0xBE, 0xA2, 0xEE, 0x6F, 0xA4, 0x58, 0x97)

TRACELOGGING_DEFINE_PROVIDER(gLogProvider, PROVIDER_NAME, PROVIVDER_GUID);

using namespace std;
using namespace experimental::filesystem;

#define WINEVENT_LEVEL_VERBOSE 5
#define WINEVENT_LEVEL_INFORMATION 4
#define WINEVENT_LEVEL_WARNING 3
#define WINEVENT_LEVEL_ERROR 2
#define WINEVENT_LEVEL_CRITICAL 1

namespace Utils
{
    ETWLogger::ETWLogger()
    {
        _exeFileName = GetExeFileName();
        TraceLoggingRegister(gLogProvider);
    }

    ETWLogger::~ETWLogger()
    {
        TraceLoggingUnregister(gLogProvider);
    }

    std::wstring ETWLogger::GetExeFileName()
    {
        wstring retValue;
        size_t size = MAX_PATH;

        do
        {
            vector<wchar_t> moduleFileName(size);
            ::GetModuleFileName(NULL, moduleFileName.data(), static_cast<DWORD>(moduleFileName.size()));

            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
            {
                size *= 2;
                continue;
            }

            path p(moduleFileName.data());
            retValue = p.filename().c_str();
            break;

        } while (true);

        return retValue;
    }

    void ETWLogger::Log(const std::wstring& msg, LoggingLevel level)
    {
        switch (level)
        {
        case Verbose:
            TraceLoggingWrite(gLogProvider, "LogMsgVerbose",
                TraceLoggingLevel(WINEVENT_LEVEL_VERBOSE), 
                TraceLoggingWideString(msg.c_str(), "msg"),
                TraceLoggingWideString(_exeFileName.c_str(), "exeName"));
            break;
        case Information:
            TraceLoggingWrite(gLogProvider, "LogMsgInformation",
                TraceLoggingLevel(WINEVENT_LEVEL_INFORMATION),
                TraceLoggingWideString(msg.c_str(), "msg"),
                TraceLoggingWideString(_exeFileName.c_str(), "exeName"));
            break;
        case Warning:
            TraceLoggingWrite(gLogProvider, "LogMsgWarning",
                TraceLoggingLevel(WINEVENT_LEVEL_WARNING),
                TraceLoggingWideString(msg.c_str(), "msg"),
                TraceLoggingWideString(_exeFileName.c_str(), "exeName"));
            break;
        case Error:
            TraceLoggingWrite(gLogProvider, "LogMsgError",
                TraceLoggingLevel(WINEVENT_LEVEL_ERROR),
                TraceLoggingWideString(msg.c_str(), "msg"),
                TraceLoggingWideString(_exeFileName.c_str(), "exeName"));
            break;
        case Critical:
            TraceLoggingWrite(gLogProvider, "LogMsgCritical",
                TraceLoggingLevel(WINEVENT_LEVEL_CRITICAL),
                TraceLoggingWideString(msg.c_str(), "msg"),
                TraceLoggingWideString(_exeFileName.c_str(), "exeName"));
            break;
        }
    }

    void ETWLogger::Log(const std::string& msg, LoggingLevel level)
    {
        switch (level)
        {
        case Verbose:
            TraceLoggingWrite(gLogProvider, "LogMsgVerbose",
                TraceLoggingLevel(WINEVENT_LEVEL_VERBOSE),
                TraceLoggingString(msg.c_str(), "msg"),
                TraceLoggingWideString(_exeFileName.c_str(), "exeName"));
            break;
        case Information:
            TraceLoggingWrite(gLogProvider, "LogMsgInformation",
                TraceLoggingLevel(WINEVENT_LEVEL_INFORMATION),
                TraceLoggingString(msg.c_str(), "msg"),
                TraceLoggingWideString(_exeFileName.c_str(), "exeName"));
            break;
        case Warning:
            TraceLoggingWrite(gLogProvider, "LogMsgWarning",
                TraceLoggingLevel(WINEVENT_LEVEL_WARNING),
                TraceLoggingString(msg.c_str(), "msg"),
                TraceLoggingWideString(_exeFileName.c_str(), "exeName"));
            break;
        case Error:
            TraceLoggingWrite(gLogProvider, "LogMsgError",
                TraceLoggingLevel(WINEVENT_LEVEL_ERROR),
                TraceLoggingString(msg.c_str(), "msg"),
                TraceLoggingWideString(_exeFileName.c_str(), "exeName"));
            break;
        case Critical:
            TraceLoggingWrite(gLogProvider, "LogMsgCritical",
                TraceLoggingLevel(WINEVENT_LEVEL_CRITICAL),
                TraceLoggingString(msg.c_str(), "msg"),
                TraceLoggingWideString(_exeFileName.c_str(), "exeName"));
            break;
        }
    }
}