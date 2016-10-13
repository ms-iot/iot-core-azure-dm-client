#pragma once

#include <vector>
#include <string>

class UpdateCSP
{
public:
    static void GetInstalledUpdates(std::vector<std::wstring>& guids);

    static void GetApprovedUpdates(std::vector<std::wstring>& guids);
    static void AddApprovedUpdates(const std::wstring& guid);

    static void GetFailedUpdates(std::vector<std::wstring>& guids);

    static void GetInstallableUpdates(std::vector<std::wstring>& guids);
    static void GetPendingRebootUpdates(std::vector<std::wstring>& guids);
    static std::wstring GetLastSuccessfulScanTime();
    static bool GetDeferUpgrade();

private:
#if _DEBUG
    static void LogGuids(const std::vector<std::wstring>& guids);
#endif
};