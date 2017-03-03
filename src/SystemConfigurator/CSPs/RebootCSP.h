#pragma once
#include <string>

class RebootCSP
{

public:
    RebootCSP();

    static void ExecRebootNow(const std::wstring& lastRebootCmdTime);

    static std::wstring GetSingleScheduleTime();
    static void SetSingleScheduleTime(const std::wstring& dailyScheduleTime);

    static std::wstring GetDailyScheduleTime();
    static void SetDailyScheduleTime(const std::wstring& dailyScheduleTime);

    static std::wstring GetLastRebootCmdTime();
    static std::wstring GetLastRebootTime();

private:
    static std::wstring _lastRebootTime;
};