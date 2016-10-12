#include "stdafx.h"
#include <deque>
#include "UpdateCSP.h"
#include "MdmProvision.h"

using namespace std;

#if _DEBUG
void UpdateCSP::LogGuids(const vector<wstring>& guids)
{
    if (guids.size() > 0)
    {
        for (const auto& it : guids)
        {
            TRACEP(L"  ", it.c_str());
        }
    }
    else
    {
        TRACE(L"  <empty>\n");
    }
}
#endif

void UpdateCSP::GetInstalledUpdates(vector<wstring>& guids)
{
    TRACE(L"\n---- Get Installed Updates\n");

    wstring resultsString;
    MdmProvision::RunGetString(L"./Device/Vendor/MSFT/Update/InstalledUpdates", resultsString);
    Utils::SplitString(resultsString, L'/', guids);
#if _DEBUG
    LogGuids(guids);
#endif
}

void UpdateCSP::GetApprovedUpdates(vector<wstring>& guids)
{
    TRACE(L"\n---- Get Approved Updates\n");

    wstring resultsString;
    MdmProvision::RunGetString(L"./Device/Vendor/MSFT/Update/ApprovedUpdates", resultsString);
    Utils::SplitString(resultsString, L'/', guids);
#if _DEBUG
    LogGuids(guids);
#endif
}

void UpdateCSP::AddApprovedUpdates(const wstring& guid)
{
    TRACE(L"\n---- Add Approved Updates\n");
    MdmProvision::RunAdd(L"./Device/Vendor/MSFT/Update/ApprovedUpdates", guid);
}

void UpdateCSP::GetFailedUpdates(vector<wstring>& guids)
{
    TRACE(L"\n---- Get Failed Updates\n");

    wstring resultsString;
    MdmProvision::RunGetString(L"./Device/Vendor/MSFT/Update/FailedUpdates", resultsString);
    Utils::SplitString(resultsString, L'/', guids);
#if _DEBUG
    LogGuids(guids);
#endif
}

void UpdateCSP::GetInstallableUpdates(vector<wstring>& guids)
{
    TRACE(L"\n---- Get Installable Updates\n");

    wstring resultsString;
    MdmProvision::RunGetString(L"./Device/Vendor/MSFT/Update/InstallableUpdates", resultsString);
    Utils::SplitString(resultsString, L'/', guids);
#if _DEBUG
    LogGuids(guids);
#endif
}

void UpdateCSP::GetPendingRebootUpdates(vector<wstring>& guids)
{
    TRACE(L"\n---- Get Pending Reboot Updates\n");

    wstring resultsString;
    MdmProvision::RunGetString(L"./Device/Vendor/MSFT/Update/PendingRebootUpdates", resultsString);
    Utils::SplitString(resultsString, L'/', guids);
#if _DEBUG
    LogGuids(guids);
#endif
}

wstring UpdateCSP::GetLastSuccessfulScanTime()
{
    TRACE(L"\n---- Get Last Successful Scan Time\n");
    wstring lastScanTime = MdmProvision::RunGetString(L"./Device/Vendor/MSFT/Update/LastSuccessfulScanTime", lastScanTime);
    TRACEP(L"  ",  lastScanTime.c_str());
    return lastScanTime;
}

bool UpdateCSP::GetDeferUpgrade()
{
    TRACE(L"\n---- Get Defer Upgrade\n");
    wstring resultsString;
    MdmProvision::RunGetString(L"./Device/Vendor/MSFT/Update/DeferUpgrade", resultsString);
    TRACEP(L"  ",  resultsString.c_str());
    return (0 == _wcsicmp(L"true", resultsString.c_str()));
}

