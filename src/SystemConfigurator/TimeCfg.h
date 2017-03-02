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
    static GetTimeInfoResponse^ Get();
    static void Set(SetTimeInfoRequest^ request);

private:
    static void Get(TimeInfo& info);
    static void SetNtpServer(const std::wstring& ntpServer);
};