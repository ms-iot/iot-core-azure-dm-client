#pragma once

#include <string>
#include <vector>

class CustomDeviceUiCSP
{
public:
    static std::wstring GetStartupAppId();
    static std::wstring GetBackgroundTasksToLaunch();
    static void AddAsStartupApp(const std::wstring& appId, bool backgroundApplication);
    static void RemoveBackgroundApplicationAsStartupApp(const std::wstring& appId);
};