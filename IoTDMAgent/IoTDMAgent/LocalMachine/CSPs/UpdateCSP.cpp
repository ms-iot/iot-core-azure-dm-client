#include "stdafx.h"
#include <deque>
#include "UpdateCSP.h"
#include "MdmProvision.h"
#include "PrivateAPIs\CSPController.h"

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

std::wstring UpdateCSP::GetInstalledUpdates()
{
    TRACE(L"\n---- Get Installed Updates\n");

    wstring resultsString = MdmProvision::RunGetString(L"./Device/Vendor/MSFT/Update/InstalledUpdates");
    TRACEP(L"    :", resultsString.c_str());
    return resultsString;
}

void UpdateCSP::GetInstalledUpdates(vector<wstring>& guids)
{
    TRACE(L"\n---- Get Installed Updates\n");

    wstring resultsString = GetInstalledUpdates();
    Utils::SplitString(resultsString, L'/', guids);
#if _DEBUG
    LogGuids(guids);
#endif
}

std::wstring UpdateCSP::GetApprovedUpdates()
{
    TRACE(L"\n---- Get Approved Updates\n");

    wstring resultsString = MdmProvision::RunGetString(L"./Device/Vendor/MSFT/Update/ApprovedUpdates");
    TRACEP(L"    :", resultsString.c_str());
    return resultsString;
}

void UpdateCSP::GetApprovedUpdates(vector<wstring>& guids)
{
    TRACE(L"\n---- Get Approved Updates\n");

    wstring resultsString = GetApprovedUpdates();
    Utils::SplitString(resultsString, L'/', guids);
#if _DEBUG
    LogGuids(guids);
#endif
}

void UpdateCSP::AddApprovedUpdates(const wstring& guid)
{
    TRACE(L"\n---- Add Approved Updates\n");
    try
    {
        MdmProvision::RunAdd(L"./Device/Vendor/MSFT/Update/ApprovedUpdates", guid);
    }
    catch (const DMExceptionWithErrorCode& e)
    {
        if (e.ErrorCode() != OMADM_ERROR_ALREADY_EXISTS)
        {
            throw;
        }
    }
}

std::wstring UpdateCSP::GetFailedUpdates()
{
    TRACE(L"\n---- Get Failed Updates\n");

    wstring resultsString = MdmProvision::RunGetString(L"./Device/Vendor/MSFT/Update/FailedUpdates");
    TRACEP(L"    :", resultsString.c_str());
    return resultsString;
}

void UpdateCSP::GetFailedUpdates(vector<wstring>& guids)
{
    TRACE(L"\n---- Get Failed Updates\n");

    wstring resultsString = GetFailedUpdates();
    Utils::SplitString(resultsString, L'/', guids);
#if _DEBUG
    LogGuids(guids);
#endif
}

wstring UpdateCSP::GetInstallableUpdates()
{
    TRACE(L"\n---- Get Installable Updates\n");

    wstring resultsString = MdmProvision::RunGetString(L"./Device/Vendor/MSFT/Update/InstallableUpdates");
    TRACEP(L"    :", resultsString.c_str());
    return resultsString;
}

void UpdateCSP::GetInstallableUpdates(vector<wstring>& guids)
{
    TRACE(L"\n---- Get Installable Updates\n");

    wstring resultsString = GetInstallableUpdates();
    Utils::SplitString(resultsString, L'/', guids);
#if _DEBUG
    LogGuids(guids);
#endif
}

wstring UpdateCSP::GetPendingRebootUpdates()
{
    wstring resultsString = MdmProvision::RunGetString(L"./Device/Vendor/MSFT/Update/PendingRebootUpdates");
    TRACEP(L"    :", resultsString.c_str());
    return resultsString;
}

void UpdateCSP::GetPendingRebootUpdates(vector<wstring>& guids)
{
    TRACE(L"\n---- Get Pending Reboot Updates\n");

    wstring resultsString = GetPendingRebootUpdates();
    Utils::SplitString(resultsString, L'/', guids);
#if _DEBUG
    LogGuids(guids);
#endif
}

wstring UpdateCSP::GetLastSuccessfulScanTime()
{
    TRACE(L"\n---- Get Last Successful Scan Time\n");
    wstring lastScanTime = MdmProvision::RunGetString(L"./Device/Vendor/MSFT/Update/LastSuccessfulScanTime");
    TRACEP(L"  ",  lastScanTime.c_str());
    return lastScanTime;
}

bool UpdateCSP::GetDeferUpgrade()
{
    TRACE(L"\n---- Get Defer Upgrade\n");
    wstring resultsString = MdmProvision::RunGetString(L"./Device/Vendor/MSFT/Update/DeferUpgrade");
    TRACEP(L"  ",  resultsString.c_str());
    return (0 == _wcsicmp(L"true", resultsString.c_str()));
}

