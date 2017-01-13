#pragma once
#include <string>

class RebootCSP
{
public:
    static void ExecRebootNow();

    static std::wstring GetSingleScheduleTime();
    static void SetSingleScheduleTime(const std::wstring& dailyScheduleTime);

    static std::wstring GetDailyScheduleTime();
    static void SetDailyScheduleTime(const std::wstring& dailyScheduleTime);
};