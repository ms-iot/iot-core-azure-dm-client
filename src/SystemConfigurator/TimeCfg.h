#pragma once

#include <string>
#include <windows.h>

class TimeCfg
{
    struct TimeInfo
    {
        std::wstring localTime;
        std::wstring ntpServer;
        TIME_ZONE_INFORMATION timeZoneInformation;
    };

public:
    static std::wstring GetTimeInfoJson();
    static void SetTimeInfo(const std::wstring& jsonString);

private:
    static void GetTimeInfo(TimeInfo& timeInfo);
    static void SetNtpServer(const std::wstring& ntpServer);
};