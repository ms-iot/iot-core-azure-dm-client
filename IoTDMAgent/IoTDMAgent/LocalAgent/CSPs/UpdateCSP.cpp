#include "stdafx.h"
#include <deque>
#include "UpdateCSP.h"
#include "..\..\Utilities\Utils.h"
#include "..\..\Utilities\Logger.h"
#include "MdmProvision.h"

using namespace std;

#if _DEBUG
void UpdateCSP::LogGuids(const vector<wstring> guids)
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

bool UpdateCSP::GetInstalledUpdates(vector<wstring>& guids)
{
    TRACE(L"\n---- Get Installed Updates\n");

    wstring resultsString;
    if (!MdmProvision::RunGet(L"./Device/Vendor/MSFT/Update/InstalledUpdates", resultsString))
    {
        return false;
    }

    if (resultsString.length() > 0)
    {
        Utils::SplitString(resultsString, L'/', guids);
    }
#if _DEBUG
    LogGuids(guids);
#endif
    return true;
}

bool UpdateCSP::GetApprovedUpdates(vector<wstring>& guids)
{
    TRACE(L"\n---- Get Approved Updates\n");

    wstring resultsString;
    if (!MdmProvision::RunGet(L"./Device/Vendor/MSFT/Update/ApprovedUpdates", resultsString))
    {
        return false;
    }

    if (resultsString.length() > 0)
    {
        Utils::SplitString(resultsString, L'/', guids);
    }

#if _DEBUG
    LogGuids(guids);
#endif
    return true;
}

bool UpdateCSP::AddApprovedUpdates(const wstring& guid)
{
    TRACE(L"\n---- Add Approved Updates\n");
    if (guid.length() > 36)
    {
        TRACE(L"Error: invalid guid to add!\n");
        return false;
    }

    wstring value;
    if (!MdmProvision::RunAdd(L"./Device/Vendor/MSFT/Update/ApprovedUpdates", value))
    {
        return false;
    }

    return true;
}

bool UpdateCSP::GetFailedUpdates(vector<wstring>& guids)
{
    TRACE(L"\n---- Get Failed Updates\n");

    wstring resultsString;
    if (!MdmProvision::RunGet(L"./Device/Vendor/MSFT/Update/FailedUpdates", resultsString))
    {
        return false;
    }

    if (resultsString.length() > 0)
    {
        Utils::SplitString(resultsString, L'/', guids);
    }

#if _DEBUG
    LogGuids(guids);
#endif

    return true;
}

bool UpdateCSP::GetInstallableUpdates(vector<wstring>& guids)
{
    TRACE(L"\n---- Get Installable Updates\n");

    wstring resultsString;
    if (!MdmProvision::RunGet(L"./Device/Vendor/MSFT/Update/InstallableUpdates", resultsString))
    {
        return false;
    }

    if (resultsString.length() > 0)
    {
        Utils::SplitString(resultsString, L'/', guids);
    }

#if _DEBUG
    LogGuids(guids);
#endif

    return true;
}

bool UpdateCSP::GetPendingRebootUpdates(vector<wstring>& guids)
{
    TRACE(L"\n---- Get Pending Reboot Updates\n");

    wstring resultsString;
    if (!MdmProvision::RunGet(L"./Device/Vendor/MSFT/Update/PendingRebootUpdates", resultsString))
    {
        return false;
    }

    if (resultsString.length() > 0)
    {
        Utils::SplitString(resultsString, L'/', guids);
    }

#if _DEBUG
    LogGuids(guids);
#endif

    return true;
}

bool UpdateCSP::GetLastSuccessfulScanTime(wstring& lastScanTime)
{
    TRACE(L"\n---- Get Last Successful Scan Time\n");

    if (MdmProvision::RunGet(L"./Device/Vendor/MSFT/Update/LastSuccessfulScanTime", lastScanTime))
    {
        TRACEP(L"  ",  lastScanTime.c_str());
        return true;
    }

    return false;
}

bool UpdateCSP::GetDeferUpgrade(bool& deferUpgrade)
{
    TRACE(L"\n---- Get Defer Upgrade\n");

    wstring resultsString;
    if (MdmProvision::RunGet(L"./Device/Vendor/MSFT/Update/DeferUpgrade", resultsString))
    {
        TRACEP(L"  ",  resultsString.c_str());
        deferUpgrade = (0 == _wcsicmp(L"true", resultsString.c_str()));
        return true;
    }

    return false;
}

