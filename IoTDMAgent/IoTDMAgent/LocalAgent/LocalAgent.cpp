#include "stdafx.h"
#include "LocalAgent.h"
#include "..\Utilities\Utils.h"
#include "..\Utilities\Logger.h"
#include "CSPs\UpdateCSP.h"
#include "CSPs\PolicyCSP.h"
#include  <algorithm>

using namespace std;

void LocalAgent::Reboot()
{
    TRACE("LocalAgent::OnDeviceRebootExecute()");

    HANDLE hToken;
    TOKEN_PRIVILEGES tkp;

    // Get a token for this process. 
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
    {
        TRACE("Error: Failed to open process token...");
        return;
    }

    // Get the LUID for the shutdown privilege. 
    if (!LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid))
    {
        TRACE("Error: Failed to look up privilege value...");
        return;
    }

    tkp.PrivilegeCount = 1;  // one privilege to set    
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    // Get the shutdown privilege for this process. 
    AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);
    if (GetLastError() != ERROR_SUCCESS)
    {
        TRACE("Error: Failed to adjust process token privileges...");
        return;
    }

    if (!InitiateSystemShutdownEx(
        NULL,   // machine name
        NULL,   // message
        10,     // timeout in seconds
        TRUE,   // force restart without waiting for apps to save
        TRUE,   // restart
        SHTDN_REASON_MAJOR_OPERATINGSYSTEM | SHTDN_REASON_MINOR_MAINTENANCE | SHTDN_REASON_FLAG_PLANNED))
    {
        char msg[512];
        sprintf_s(msg, "Error: Reboot failed to be scheduled.! Error code = %d", GetLastError());
        TRACE(msg);
    }
    else
    {
        TRACE("Reboot scheduled successfully.");
    }

    return;
}

unsigned int LocalAgent::GetTotalMemoryMB()
{
    TRACE("LocalAgent::GetTotalMemoryMB()");

    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    GlobalMemoryStatusEx(&statex);
    return static_cast<unsigned int>(statex.ullTotalPhys) / (1024 * 1024);
}

unsigned int LocalAgent::GetAvailableMemoryMB()
{
    TRACE("LocalAgent::GetAvailableMemoryKB()");

    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    GlobalMemoryStatusEx(&statex);
    return static_cast<unsigned int>(statex.ullAvailPhys) / (1024 * 1024);
}

unsigned int LocalAgent::GetBatteryLevel()
{
    // ToDo: NotImpl
    TRACE("LocalAgent::GetBatteryLevel()");

    static unsigned int level = 100;
    if (level >= 10)
    {
        level -= 10;
    }
    else
    {
        level = 100;
    }
    return level;
}

unsigned int LocalAgent::GetBatteryStatus()
{
    // ToDo: NotImpl
    TRACE("LocalAgent::GetBatteryStatus()");
    return 2;
}

bool LocalAgent::RunSyncML(const wstring& request, wstring& response)
{
    wstring sid = L""; // only supports device-wide operations for now.
    return MdmProvision::RunSyncML(sid, request, response);
}

bool LocalAgent::RunSyncMLAdd(const wstring& path, wstring& value)
{
    wstring sid = L""; // only supports device-wide operations for now.
    return MdmProvision::RunAdd(sid, path, value);
}

bool LocalAgent::RunSyncMLGet(const wstring& path, wstring& value)
{
    wstring sid = L""; // only supports device-wide operations for now.
    return MdmProvision::RunGet(sid, path, value);
}

bool LocalAgent::RunSyncMLGet(const wstring& path, unsigned int& value)
{
    wstring sid = L""; // only supports device-wide operations for now.
    return MdmProvision::RunGet(sid, path, value);
}

bool LocalAgent::RunSyncMLSet(const wstring& path, const wstring& value)
{
    wstring sid = L""; // only supports device-wide operations for now.
    return MdmProvision::RunSet(sid, path, value);
}

bool LocalAgent::RunSyncMLSet(const wstring& path, unsigned int value)
{
    wstring sid = L""; // only supports device-wide operations for now.
    return MdmProvision::RunSet(sid, path, value);
}

bool LocalAgent::GetInstalledUpdates(vector<wstring>& guids)
{
    return UpdateCSP::GetInstalledUpdates(guids);
}

bool LocalAgent::GetApprovedUpdates(vector<wstring>& guids)
{
    return UpdateCSP::GetInstalledUpdates(guids);
}

bool LocalAgent::AddApprovedUpdates(const wstring& guid)
{
    return UpdateCSP::AddApprovedUpdates(guid);
}

bool LocalAgent::GetFailedUpdates(vector<wstring>& guids)
{
    return UpdateCSP::GetFailedUpdates(guids);
}

bool LocalAgent::GetInstallableUpdates(vector<wstring>& guids)
{
    return UpdateCSP::GetInstallableUpdates(guids);
}

bool LocalAgent::GetPendingRebootUpdates(vector<wstring>& guids)
{
    return UpdateCSP::GetPendingRebootUpdates(guids);
}

bool LocalAgent::GetLastSuccessfulScanTime(wstring& lastScanTime)
{
    return UpdateCSP::GetLastSuccessfulScanTime(lastScanTime);
}

bool LocalAgent::GetDeferUpgrade(bool& deferUpgrade)
{
    return UpdateCSP::GetDeferUpgrade(deferUpgrade);
}

bool LocalAgent::GetActiveHoursStart(unsigned int& hours)
{
    return PolicyCSP::GetActiveHoursStart(hours);
}

bool LocalAgent::SetActiveHoursStart(unsigned int hours)
{
    return PolicyCSP::SetActiveHoursStart(hours);
}

bool LocalAgent::GetActiveHoursEnd(unsigned int& hours)
{
    return PolicyCSP::GetActiveHoursEnd(hours);
}

bool LocalAgent::SetActiveHoursEnd(unsigned int hours)
{
    return PolicyCSP::SetActiveHoursEnd(hours);
}

bool LocalAgent::GetAllowAutoUpdate(unsigned int& allowLevel)
{
    return PolicyCSP::GetAllowAutoUpdate(allowLevel);
}

bool LocalAgent::SetAllowAutoUpdate(unsigned int allowLevel)
{
    return PolicyCSP::SetAllowAutoUpdate(allowLevel);
}

bool LocalAgent::GetAllowMUUpdateService(unsigned int& allowLevel)
{
    return PolicyCSP::GetAllowMUUpdateService(allowLevel);
}

bool LocalAgent::SetAllowMUUpdateService(unsigned int allowLevel)
{
    return PolicyCSP::SetAllowMUUpdateService(allowLevel);
}

bool LocalAgent::GetAllowNonMicrosoftSignedUpdate(unsigned int& allowLevel)
{
    return PolicyCSP::GetAllowNonMicrosoftSignedUpdate(allowLevel);
}

bool LocalAgent::SetAllowNonMicrosoftSignedUpdate(unsigned int allowLevel)
{
    return PolicyCSP::SetAllowNonMicrosoftSignedUpdate(allowLevel);
}

bool LocalAgent::GetAllowUpdateService(unsigned int& allowLevel)
{
    return PolicyCSP::GetAllowUpdateService(allowLevel);
}

bool LocalAgent::SetAllowUpdateService(unsigned int allowLevel)
{
    return PolicyCSP::SetAllowUpdateService(allowLevel);
}

bool LocalAgent::GetBranchReadinessLevel(unsigned int& level)
{
    return PolicyCSP::GetBranchReadinessLevel(level);
}

bool LocalAgent::SetBranchReadinessLevel(unsigned int level)
{
    return PolicyCSP::SetBranchReadinessLevel(level);
}

bool LocalAgent::GetDeferFeatureUpdatesPeriodInDays(unsigned int& days)
{
    return PolicyCSP::GetDeferFeatureUpdatesPeriodInDays(days);
}

bool LocalAgent::SetDeferFeatureUpdatesPeriodInDays(unsigned int days)
{
    return PolicyCSP::SetDeferFeatureUpdatesPeriodInDays(days);
}

bool LocalAgent::GetDeferQualityUpdatesPeriodInDays(unsigned int& days)
{
    return PolicyCSP::GetDeferQualityUpdatesPeriodInDays(days);
}

bool LocalAgent::SetDeferQualityUpdatesPeriodInDays(unsigned int days)
{
    return PolicyCSP::SetDeferQualityUpdatesPeriodInDays(days);
}

bool LocalAgent::GetDeferUpdatePeriod(unsigned int& weeks)
{
    return PolicyCSP::GetDeferUpdatePeriod(weeks);
}

bool LocalAgent::SetDeferUpdatePeriod(unsigned int weeks)
{
    return PolicyCSP::SetDeferUpdatePeriod(weeks);
}

bool LocalAgent::GetDeferUpgradePeriod(unsigned int& months)
{
    return PolicyCSP::GetDeferUpgradePeriod(months);
}

bool LocalAgent::SetDeferUpgradePeriod(unsigned int months)
{
    return PolicyCSP::SetDeferUpgradePeriod(months);
}

bool LocalAgent::GetExcludeWUDrivers(unsigned int& excludeLevel)
{
    return PolicyCSP::GetExcludeWUDrivers(excludeLevel);
}

bool LocalAgent::SetExcludeWUDrivers(unsigned int excludeLevel)
{
    return PolicyCSP::SetExcludeWUDrivers(excludeLevel);
}

bool LocalAgent::GetPauseDeferrals(unsigned int& pauseLevel)
{
    return PolicyCSP::GetPauseDeferrals(pauseLevel);
}

bool LocalAgent::SetPauseDeferrals(unsigned int pauseLevel)
{
    return PolicyCSP::SetPauseDeferrals(pauseLevel);
}

bool LocalAgent::GetPauseFeatureUpdates(unsigned int& pauseLevel)
{
    return PolicyCSP::GetPauseFeatureUpdates(pauseLevel);
}

bool LocalAgent::SetPauseFeatureUpdates(unsigned int pauseLevel)
{
    return PolicyCSP::SetPauseFeatureUpdates(pauseLevel);
}

bool LocalAgent::GetPauseQualityUpdates(unsigned int& pauseLevel)
{
    return PolicyCSP::GetPauseQualityUpdates(pauseLevel);
}

bool LocalAgent::SetPauseQualityUpdates(unsigned int pauseLevel)
{
    return PolicyCSP::SetPauseQualityUpdates(pauseLevel);
}

bool LocalAgent::GetRequireDeferUpgrade(unsigned int& value)
{
    return PolicyCSP::GetRequireDeferUpgrade(value);
}

bool LocalAgent::SetRequireDeferUpgrade(unsigned int value)
{
    return PolicyCSP::SetRequireDeferUpgrade(value);
}

bool LocalAgent::GetRequireUpdateApproval(unsigned int& value)
{
    return PolicyCSP::GetRequireUpdateApproval(value);
}

bool LocalAgent::SetRequireUpdateApproval(unsigned int value)
{
    return PolicyCSP::SetRequireUpdateApproval(value);
}

bool LocalAgent::GetScheduledInstallDay(unsigned int& dayOfWeek)
{
    return PolicyCSP::GetScheduledInstallDay(dayOfWeek);
}

bool LocalAgent::SetScheduledInstallDay(unsigned int dayOfWeek)
{
    return PolicyCSP::SetScheduledInstallDay(dayOfWeek);
}

bool LocalAgent::GetScheduledInstallTime(unsigned int& hour)
{
    return PolicyCSP::GetScheduledInstallTime(hour);
}

bool LocalAgent::SetScheduledInstallTime(unsigned int hour)
{
    return PolicyCSP::SetScheduledInstallTime(hour);
}

bool LocalAgent::GetUpdateServiceUrl(std::wstring& serviceUrl)
{
    return PolicyCSP::GetUpdateServiceUrl(serviceUrl);
}

bool LocalAgent::SetUpdateServiceUrl(const std::wstring& serviceUrl)
{
    return PolicyCSP::SetUpdateServiceUrl(serviceUrl);
}
