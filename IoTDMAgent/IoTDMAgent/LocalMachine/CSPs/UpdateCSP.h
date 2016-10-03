#pragma once

#include <vector>
#include <string>
#include "MdmProvision.h"

class UpdateCSP
{
public:
    static bool GetInstalledUpdates(std::vector<std::wstring>& guids);

    static bool GetApprovedUpdates(std::vector<std::wstring>& guids);
    static bool AddApprovedUpdates(const std::wstring& guid);

    static bool GetFailedUpdates(std::vector<std::wstring>& guids);

    static bool GetInstallableUpdates(std::vector<std::wstring>& guids);
    static bool GetPendingRebootUpdates(std::vector<std::wstring>& guids);
    static bool GetLastSuccessfulScanTime(std::wstring& lastScanTime);
    static bool GetDeferUpgrade(bool& deferUpgrade);

private:
#if _DEBUG
    static void LogGuids(const std::vector<std::wstring>& guids);
#endif
};