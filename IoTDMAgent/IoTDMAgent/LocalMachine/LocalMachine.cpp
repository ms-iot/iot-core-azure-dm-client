#include "stdafx.h"
#include "LocalMachine.h"
#include "..\Utilities\Utils.h"
#include "..\Utilities\Logger.h"
#include "CSPs\UpdateCSP.h"
#include "CSPs\PolicyCSP.h"
#include  <algorithm>

using namespace std;

void LocalMachine::Reboot()
{
    TRACE("LocalMachine::OnDeviceRebootExecute()");

    HANDLE hToken;

    // Get a token for this process. 
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
    {
        TRACE("Error: Failed to open process token...");
        return;
    }

    // Get the LUID for the shutdown privilege. 
    TOKEN_PRIVILEGES tkp;
    if (LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid))
    {
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
    }
    else
    {
        TRACE("Error: Failed to look up privilege value...");
    }

    CloseHandle(hToken);
}

unsigned int LocalMachine::GetTotalMemoryMB()
{
    TRACE("LocalMachine::GetTotalMemoryMB()");

    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    GlobalMemoryStatusEx(&statex);
    return static_cast<unsigned int>(statex.ullTotalPhys) / (1024 * 1024);
}

unsigned int LocalMachine::GetAvailableMemoryMB()
{
    TRACE("LocalMachine::GetAvailableMemoryKB()");

    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    GlobalMemoryStatusEx(&statex);
    return static_cast<unsigned int>(statex.ullAvailPhys) / (1024 * 1024);
}

unsigned int LocalMachine::GetBatteryLevel()
{
    // ToDo: NotImpl
    TRACE("LocalMachine::GetBatteryLevel()");

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

unsigned int LocalMachine::GetBatteryStatus()
{
    // ToDo: NotImpl
    TRACE("LocalMachine::GetBatteryStatus()");
    return 2;
}

bool LocalMachine::RunSyncML(const wstring& request, wstring& response)
{
    wstring sid = L""; // only supports device-wide operations for now.
    return MdmProvision::RunSyncML(sid, request, response);
}

bool LocalMachine::RunSyncMLAdd(const wstring& path, wstring& value)
{
    wstring sid = L""; // only supports device-wide operations for now.
    return MdmProvision::RunAdd(sid, path, value);
}

bool LocalMachine::RunSyncMLGet(const wstring& path, wstring& value)
{
    wstring sid = L""; // only supports device-wide operations for now.
    return MdmProvision::RunGet(sid, path, value);
}

bool LocalMachine::RunSyncMLGet(const wstring& path, unsigned int& value)
{
    wstring sid = L""; // only supports device-wide operations for now.
    return MdmProvision::RunGet(sid, path, value);
}

bool LocalMachine::RunSyncMLSet(const wstring& path, const wstring& value)
{
    wstring sid = L""; // only supports device-wide operations for now.
    return MdmProvision::RunSet(sid, path, value);
}

bool LocalMachine::RunSyncMLSet(const wstring& path, unsigned int value)
{
    wstring sid = L""; // only supports device-wide operations for now.
    return MdmProvision::RunSet(sid, path, value);
}

bool LocalMachine::GetInstalledUpdates(vector<wstring>& guids)
{
    return UpdateCSP::GetInstalledUpdates(guids);
}

bool LocalMachine::GetApprovedUpdates(vector<wstring>& guids)
{
    return UpdateCSP::GetInstalledUpdates(guids);
}

bool LocalMachine::AddApprovedUpdates(const wstring& guid)
{
    return UpdateCSP::AddApprovedUpdates(guid);
}

bool LocalMachine::GetFailedUpdates(vector<wstring>& guids)
{
    return UpdateCSP::GetFailedUpdates(guids);
}

bool LocalMachine::GetInstallableUpdates(vector<wstring>& guids)
{
    return UpdateCSP::GetInstallableUpdates(guids);
}

bool LocalMachine::GetPendingRebootUpdates(vector<wstring>& guids)
{
    return UpdateCSP::GetPendingRebootUpdates(guids);
}

bool LocalMachine::GetLastSuccessfulScanTime(wstring& lastScanTime)
{
    return UpdateCSP::GetLastSuccessfulScanTime(lastScanTime);
}

bool LocalMachine::GetDeferUpgrade(bool& deferUpgrade)
{
    return UpdateCSP::GetDeferUpgrade(deferUpgrade);
}

bool LocalMachine::GetActiveHoursStart(unsigned int& hours)
{
    return PolicyCSP::GetActiveHoursStart(hours);
}

bool LocalMachine::SetActiveHoursStart(unsigned int hours)
{
    return PolicyCSP::SetActiveHoursStart(hours);
}

bool LocalMachine::GetActiveHoursEnd(unsigned int& hours)
{
    return PolicyCSP::GetActiveHoursEnd(hours);
}

bool LocalMachine::SetActiveHoursEnd(unsigned int hours)
{
    return PolicyCSP::SetActiveHoursEnd(hours);
}

bool LocalMachine::GetAllowAutoUpdate(unsigned int& allowLevel)
{
    return PolicyCSP::GetAllowAutoUpdate(allowLevel);
}

bool LocalMachine::SetAllowAutoUpdate(unsigned int allowLevel)
{
    return PolicyCSP::SetAllowAutoUpdate(allowLevel);
}

bool LocalMachine::GetAllowMUUpdateService(unsigned int& allowLevel)
{
    return PolicyCSP::GetAllowMUUpdateService(allowLevel);
}

bool LocalMachine::SetAllowMUUpdateService(unsigned int allowLevel)
{
    return PolicyCSP::SetAllowMUUpdateService(allowLevel);
}

bool LocalMachine::GetAllowNonMicrosoftSignedUpdate(unsigned int& allowLevel)
{
    return PolicyCSP::GetAllowNonMicrosoftSignedUpdate(allowLevel);
}

bool LocalMachine::SetAllowNonMicrosoftSignedUpdate(unsigned int allowLevel)
{
    return PolicyCSP::SetAllowNonMicrosoftSignedUpdate(allowLevel);
}

bool LocalMachine::GetAllowUpdateService(unsigned int& allowLevel)
{
    return PolicyCSP::GetAllowUpdateService(allowLevel);
}

bool LocalMachine::SetAllowUpdateService(unsigned int allowLevel)
{
    return PolicyCSP::SetAllowUpdateService(allowLevel);
}

bool LocalMachine::GetBranchReadinessLevel(unsigned int& level)
{
    return PolicyCSP::GetBranchReadinessLevel(level);
}

bool LocalMachine::SetBranchReadinessLevel(unsigned int level)
{
    return PolicyCSP::SetBranchReadinessLevel(level);
}

bool LocalMachine::GetDeferFeatureUpdatesPeriodInDays(unsigned int& days)
{
    return PolicyCSP::GetDeferFeatureUpdatesPeriodInDays(days);
}

bool LocalMachine::SetDeferFeatureUpdatesPeriodInDays(unsigned int days)
{
    return PolicyCSP::SetDeferFeatureUpdatesPeriodInDays(days);
}

bool LocalMachine::GetDeferQualityUpdatesPeriodInDays(unsigned int& days)
{
    return PolicyCSP::GetDeferQualityUpdatesPeriodInDays(days);
}

bool LocalMachine::SetDeferQualityUpdatesPeriodInDays(unsigned int days)
{
    return PolicyCSP::SetDeferQualityUpdatesPeriodInDays(days);
}

bool LocalMachine::GetDeferUpdatePeriod(unsigned int& weeks)
{
    return PolicyCSP::GetDeferUpdatePeriod(weeks);
}

bool LocalMachine::SetDeferUpdatePeriod(unsigned int weeks)
{
    return PolicyCSP::SetDeferUpdatePeriod(weeks);
}

bool LocalMachine::GetDeferUpgradePeriod(unsigned int& months)
{
    return PolicyCSP::GetDeferUpgradePeriod(months);
}

bool LocalMachine::SetDeferUpgradePeriod(unsigned int months)
{
    return PolicyCSP::SetDeferUpgradePeriod(months);
}

bool LocalMachine::GetExcludeWUDrivers(unsigned int& excludeLevel)
{
    return PolicyCSP::GetExcludeWUDrivers(excludeLevel);
}

bool LocalMachine::SetExcludeWUDrivers(unsigned int excludeLevel)
{
    return PolicyCSP::SetExcludeWUDrivers(excludeLevel);
}

bool LocalMachine::GetPauseDeferrals(unsigned int& pauseLevel)
{
    return PolicyCSP::GetPauseDeferrals(pauseLevel);
}

bool LocalMachine::SetPauseDeferrals(unsigned int pauseLevel)
{
    return PolicyCSP::SetPauseDeferrals(pauseLevel);
}

bool LocalMachine::GetPauseFeatureUpdates(unsigned int& pauseLevel)
{
    return PolicyCSP::GetPauseFeatureUpdates(pauseLevel);
}

bool LocalMachine::SetPauseFeatureUpdates(unsigned int pauseLevel)
{
    return PolicyCSP::SetPauseFeatureUpdates(pauseLevel);
}

bool LocalMachine::GetPauseQualityUpdates(unsigned int& pauseLevel)
{
    return PolicyCSP::GetPauseQualityUpdates(pauseLevel);
}

bool LocalMachine::SetPauseQualityUpdates(unsigned int pauseLevel)
{
    return PolicyCSP::SetPauseQualityUpdates(pauseLevel);
}

bool LocalMachine::GetRequireDeferUpgrade(unsigned int& value)
{
    return PolicyCSP::GetRequireDeferUpgrade(value);
}

bool LocalMachine::SetRequireDeferUpgrade(unsigned int value)
{
    return PolicyCSP::SetRequireDeferUpgrade(value);
}

bool LocalMachine::GetRequireUpdateApproval(unsigned int& value)
{
    return PolicyCSP::GetRequireUpdateApproval(value);
}

bool LocalMachine::SetRequireUpdateApproval(unsigned int value)
{
    return PolicyCSP::SetRequireUpdateApproval(value);
}

bool LocalMachine::GetScheduledInstallDay(unsigned int& dayOfWeek)
{
    return PolicyCSP::GetScheduledInstallDay(dayOfWeek);
}

bool LocalMachine::SetScheduledInstallDay(unsigned int dayOfWeek)
{
    return PolicyCSP::SetScheduledInstallDay(dayOfWeek);
}

bool LocalMachine::GetScheduledInstallTime(unsigned int& hour)
{
    return PolicyCSP::GetScheduledInstallTime(hour);
}

bool LocalMachine::SetScheduledInstallTime(unsigned int hour)
{
    return PolicyCSP::SetScheduledInstallTime(hour);
}

bool LocalMachine::GetUpdateServiceUrl(std::wstring& serviceUrl)
{
    return PolicyCSP::GetUpdateServiceUrl(serviceUrl);
}

bool LocalMachine::SetUpdateServiceUrl(const std::wstring& serviceUrl)
{
    return PolicyCSP::SetUpdateServiceUrl(serviceUrl);
}
