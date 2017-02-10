#pragma once

#include <string>
#include <windows.h>
#include "Models\TimeInfo.h"

using namespace Microsoft::Devices::Management::Message;

class TimeCfg
{
    struct TimeInfo
    {
        std::wstring localTime;
        std::wstring ntpServer;
        TIME_ZONE_INFORMATION timeZoneInformation;
    };

public:
    static TimeInfoResponse^ GetTimeInfo();
    static void SetTimeInfo(IRequest^ request);

private:
    static void GetTimeInfo(TimeInfo& timeInfo);
    static void SetNtpServer(const std::wstring& ntpServer);
};