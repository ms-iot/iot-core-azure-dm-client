#pragma once

#include <string>
#include <windows.h>

class AppCfg
{
public:
    static void StartApp(const std::wstring& appId) { StartStopApp(appId, true); }
    static void StopApp(const std::wstring& appId) { StartStopApp(appId, false); }
private:
    static void StartStopApp(const std::wstring& appId, bool start);
};