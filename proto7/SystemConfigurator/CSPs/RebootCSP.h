#pragma once
#include <string>

class RebootCSP
{

public:
    RebootCSP();

    static void ExecRebootNow();

    static void SetRebootInfo(const std::wstring& jsonString);
    static std::wstring GetRebootInfoJson();

    static std::wstring GetSingleScheduleTime();
    static void SetSingleScheduleTime(const std::wstring& dailyScheduleTime);

    static std::wstring GetDailyScheduleTime();
    static void SetDailyScheduleTime(const std::wstring& dailyScheduleTime);

    static std::wstring GetLastRebootCmdTime();
    static std::wstring GetLastRebootTime();

private:
    static std::wstring _lastRebootTime;
};