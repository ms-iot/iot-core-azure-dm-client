#include "stdafx.h"
#include <windows.h>
#include "LocalMachine.h"
#include "..\Utilities\Utils.h"
#include "..\Utilities\Logger.h"
#include "..\Utilities\DMException.h"
#include "CSPs\UpdateCSP.h"
#include "CSPs\PolicyCSP.h"
#include  <algorithm>

using namespace std;

void LocalMachine::Reboot()
{
    TRACE("LocalMachine::OnDeviceRebootExecute()");

    HANDLE processToken;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &processToken))
    {
        throw DMException("Failed to open process token...");
    }

    // Get the LUID for the shutdown privilege. 
    TOKEN_PRIVILEGES tkp;
    if (LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid))
    {
        tkp.PrivilegeCount = 1;  // one privilege to set    
        tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

        // Get the shutdown privilege for this process. 
        if (AdjustTokenPrivileges(processToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0) && GetLastError() == ERROR_SUCCESS)
        {
            if (!InitiateSystemShutdownEx(
                NULL,   // machine name
                NULL,   // message
                10,     // timeout in seconds
                TRUE,   // force restart without waiting for apps to save
                TRUE,   // restart
                SHTDN_REASON_MAJOR_OPERATINGSYSTEM | SHTDN_REASON_MINOR_MAINTENANCE | SHTDN_REASON_FLAG_PLANNED))
            {
                TRACEP("Error: Reboot failed to be scheduled.! Error code = ", GetLastError());
            }
            else
            {
                TRACE("Reboot scheduled successfully.");
            }
        }
        else
        {
            // ToDo: Need to implement a mechanism for reporting errors from the Azure DM thread to 
            //       the service/Azure reported properties.
            TRACE("Error: Failed to adjust process token privileges...");
        }
    }
    else
    {
        // ToDo: Need to implement a mechanism for reporting errors from the Azure DM thread to 
        //       the service/Azure reported properties.
        TRACE("Error: Failed to look up privilege value...");
    }

    CloseHandle(processToken);
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

void LocalMachine::RunSyncML(const wstring& request, wstring& response)
{
    wstring sid = L""; // only supports device-wide operations for now.
    MdmProvision::RunSyncML(sid, request, response);
}

void LocalMachine::RunSyncMLAdd(const wstring& path, const wstring& value)
{
    wstring sid = L""; // only supports device-wide operations for now.
    MdmProvision::RunAdd(sid, path, value);
}

wstring LocalMachine::RunSyncMLGetString(const wstring& path)
{
    wstring sid = L""; // only supports device-wide operations for now.
    return MdmProvision::RunGetString(sid, path);
}

unsigned int LocalMachine::RunSyncMLGetInt(const wstring& path)
{
    wstring sid = L""; // only supports device-wide operations for now.
    return MdmProvision::RunGetUInt(sid, path);
}

void LocalMachine::RunSyncMLSet(const wstring& path, const wstring& value)
{
    wstring sid = L""; // only supports device-wide operations for now.
    MdmProvision::RunSet(sid, path, value);
}

void LocalMachine::RunSyncMLSet(const wstring& path, unsigned int value)
{
    wstring sid = L""; // only supports device-wide operations for now.
    MdmProvision::RunSet(sid, path, value);
}

void LocalMachine::GetInstalledUpdates(vector<wstring>& guids)
{
    UpdateCSP::GetInstalledUpdates(guids);
}

void LocalMachine::GetApprovedUpdates(vector<wstring>& guids)
{
    UpdateCSP::GetInstalledUpdates(guids);
}

void LocalMachine::AddApprovedUpdates(const wstring& guid)
{
    UpdateCSP::AddApprovedUpdates(guid);
}

void LocalMachine::GetFailedUpdates(vector<wstring>& guids)
{
    UpdateCSP::GetFailedUpdates(guids);
}

void LocalMachine::GetInstallableUpdates(vector<wstring>& guids)
{
    UpdateCSP::GetInstallableUpdates(guids);
}

void LocalMachine::GetPendingRebootUpdates(vector<wstring>& guids)
{
    UpdateCSP::GetPendingRebootUpdates(guids);
}

wstring LocalMachine::GetLastSuccessfulScanTime()
{
    return UpdateCSP::GetLastSuccessfulScanTime();
}

bool LocalMachine::GetDeferUpgrade()
{
    return UpdateCSP::GetDeferUpgrade();
}

unsigned int LocalMachine::GetActiveHoursStart()
{
    return PolicyCSP::GetActiveHoursStart();
}

void LocalMachine::SetActiveHoursStart(unsigned int hours)
{
    PolicyCSP::SetActiveHoursStart(hours);
}

unsigned int LocalMachine::GetActiveHoursEnd()
{
    return PolicyCSP::GetActiveHoursEnd();
}

void LocalMachine::SetActiveHoursEnd(unsigned int hours)
{
    PolicyCSP::SetActiveHoursEnd(hours);
}

unsigned int LocalMachine::GetAllowAutoUpdate()
{
    return PolicyCSP::GetAllowAutoUpdate();
}

void LocalMachine::SetAllowAutoUpdate(unsigned int allowLevel)
{
    PolicyCSP::SetAllowAutoUpdate(allowLevel);
}

unsigned int LocalMachine::GetAllowMUUpdateService()
{
    return PolicyCSP::GetAllowMUUpdateService();
}

void LocalMachine::SetAllowMUUpdateService(unsigned int allowLevel)
{
    PolicyCSP::SetAllowMUUpdateService(allowLevel);
}

unsigned int LocalMachine::GetAllowNonMicrosoftSignedUpdate()
{
    return PolicyCSP::GetAllowNonMicrosoftSignedUpdate();
}

void LocalMachine::SetAllowNonMicrosoftSignedUpdate(unsigned int allowLevel)
{
    PolicyCSP::SetAllowNonMicrosoftSignedUpdate(allowLevel);
}

unsigned int LocalMachine::GetAllowUpdateService()
{
    return PolicyCSP::GetAllowUpdateService();
}

void LocalMachine::SetAllowUpdateService(unsigned int allowLevel)
{
    PolicyCSP::SetAllowUpdateService(allowLevel);
}

unsigned int LocalMachine::GetBranchReadinessLevel()
{
    return PolicyCSP::GetBranchReadinessLevel();
}

void LocalMachine::SetBranchReadinessLevel(unsigned int level)
{
    PolicyCSP::SetBranchReadinessLevel(level);
}

unsigned int LocalMachine::GetDeferFeatureUpdatesPeriodInDays()
{
    return PolicyCSP::GetDeferFeatureUpdatesPeriodInDays();
}

void LocalMachine::SetDeferFeatureUpdatesPeriodInDays(unsigned int days)
{
    PolicyCSP::SetDeferFeatureUpdatesPeriodInDays(days);
}

unsigned int LocalMachine::GetDeferQualityUpdatesPeriodInDays()
{
    return PolicyCSP::GetDeferQualityUpdatesPeriodInDays();
}

void LocalMachine::SetDeferQualityUpdatesPeriodInDays(unsigned int days)
{
    PolicyCSP::SetDeferQualityUpdatesPeriodInDays(days);
}

unsigned int LocalMachine::GetDeferUpdatePeriod()
{
    return PolicyCSP::GetDeferUpdatePeriod();
}

void LocalMachine::SetDeferUpdatePeriod(unsigned int weeks)
{
    PolicyCSP::SetDeferUpdatePeriod(weeks);
}

unsigned int LocalMachine::GetDeferUpgradePeriod()
{
    return PolicyCSP::GetDeferUpgradePeriod();
}

void LocalMachine::SetDeferUpgradePeriod(unsigned int months)
{
    PolicyCSP::SetDeferUpgradePeriod(months);
}

unsigned int LocalMachine::GetExcludeWUDrivers()
{
    return PolicyCSP::GetExcludeWUDrivers();
}

void LocalMachine::SetExcludeWUDrivers(unsigned int excludeLevel)
{
    PolicyCSP::SetExcludeWUDrivers(excludeLevel);
}

unsigned int LocalMachine::GetPauseDeferrals()
{
    return PolicyCSP::GetPauseDeferrals();
}

void LocalMachine::SetPauseDeferrals(unsigned int pauseLevel)
{
    PolicyCSP::SetPauseDeferrals(pauseLevel);
}

unsigned int LocalMachine::GetPauseFeatureUpdates()
{
    return PolicyCSP::GetPauseFeatureUpdates();
}

void LocalMachine::SetPauseFeatureUpdates(unsigned int pauseLevel)
{
    PolicyCSP::SetPauseFeatureUpdates(pauseLevel);
}

unsigned int LocalMachine::GetPauseQualityUpdates()
{
    return PolicyCSP::GetPauseQualityUpdates();
}

void LocalMachine::SetPauseQualityUpdates(unsigned int pauseLevel)
{
    PolicyCSP::SetPauseQualityUpdates(pauseLevel);
}

unsigned int LocalMachine::GetRequireDeferUpgrade()
{
    return PolicyCSP::GetRequireDeferUpgrade();
}

void LocalMachine::SetRequireDeferUpgrade(unsigned int value)
{
    PolicyCSP::SetRequireDeferUpgrade(value);
}

unsigned int LocalMachine::GetRequireUpdateApproval()
{
    return PolicyCSP::GetRequireUpdateApproval();
}

void LocalMachine::SetRequireUpdateApproval(unsigned int value)
{
    PolicyCSP::SetRequireUpdateApproval(value);
}

unsigned int LocalMachine::GetScheduledInstallDay()
{
    return PolicyCSP::GetScheduledInstallDay();
}

void LocalMachine::SetScheduledInstallDay(unsigned int dayOfWeek)
{
    PolicyCSP::SetScheduledInstallDay(dayOfWeek);
}

unsigned int LocalMachine::GetScheduledInstallTime()
{
    return PolicyCSP::GetScheduledInstallTime();
}

void LocalMachine::SetScheduledInstallTime(unsigned int hour)
{
    PolicyCSP::SetScheduledInstallTime(hour);
}

wstring LocalMachine::GetUpdateServiceUrl()
{
    return PolicyCSP::GetUpdateServiceUrl();
}

void LocalMachine::SetUpdateServiceUrl(const std::wstring& serviceUrl)
{
    PolicyCSP::SetUpdateServiceUrl(serviceUrl);
}
