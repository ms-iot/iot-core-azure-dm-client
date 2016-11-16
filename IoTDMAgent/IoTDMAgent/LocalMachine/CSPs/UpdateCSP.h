#pragma once

#include <vector>
#include <string>

class UpdateCSP
{
public:
    static std::wstring GetInstalledUpdates();

    static std::wstring GetApprovedUpdates();
    static void AddApprovedUpdates(const std::wstring& guid);

    static std::wstring GetFailedUpdates();
    static std::wstring GetInstallableUpdates();
    static std::wstring GetPendingRebootUpdates();
    static std::wstring GetLastSuccessfulScanTime();
    static bool GetDeferUpgrade();

private:
#if _DEBUG
    static void LogGuids(const std::vector<std::wstring>& guids);
#endif
};