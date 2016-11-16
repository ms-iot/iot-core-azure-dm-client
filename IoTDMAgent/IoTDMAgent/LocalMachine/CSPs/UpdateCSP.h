#pragma once

#include <vector>
#include <string>

class UpdateCSP
{
public:
    static std::wstring GetInstalledUpdates();
    static void GetInstalledUpdates(std::vector<std::wstring>& guids);

    static std::wstring GetApprovedUpdates();
    static void GetApprovedUpdates(std::vector<std::wstring>& guids);
    static void AddApprovedUpdates(const std::wstring& guid);

    static std::wstring GetFailedUpdates();
    static void GetFailedUpdates(std::vector<std::wstring>& guids);

    static std::wstring GetInstallableUpdates();
    static void GetInstallableUpdates(std::vector<std::wstring>& guids);

    static std::wstring GetPendingRebootUpdates();
    static void GetPendingRebootUpdates(std::vector<std::wstring>& guids);

    static std::wstring GetLastSuccessfulScanTime();

    static bool GetDeferUpgrade();

private:
#if _DEBUG
    static void LogGuids(const std::vector<std::wstring>& guids);
#endif
};