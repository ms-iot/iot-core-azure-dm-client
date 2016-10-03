#pragma once

#include <string>
#include <vector>

class LocalMachine
{
public:
    static void Reboot();

    static unsigned int GetTotalMemoryMB();
    static unsigned int GetAvailableMemoryMB();
    static unsigned int GetBatteryLevel();
    static unsigned int GetBatteryStatus();

    // Generic SyncML
    static bool RunSyncML(const std::wstring& request, std::wstring& response);
    static bool RunSyncMLAdd(const std::wstring& path, std::wstring& value);
    static bool RunSyncMLGet(const std::wstring& path, std::wstring& value);
    static bool RunSyncMLGet(const std::wstring& path, unsigned int& value);
    static bool RunSyncMLSet(const std::wstring& path, const std::wstring& value);
    static bool RunSyncMLSet(const std::wstring& path, unsigned int value);

    // Update CSP
    static bool GetInstalledUpdates(std::vector<std::wstring>& guids);
    static bool GetApprovedUpdates(std::vector<std::wstring>& guids);
    static bool AddApprovedUpdates(const std::wstring& guid);
    static bool GetFailedUpdates(std::vector<std::wstring>& guids);
    static bool GetInstallableUpdates(std::vector<std::wstring>& guids);
    static bool GetPendingRebootUpdates(std::vector<std::wstring>& guids);
    static bool GetLastSuccessfulScanTime(std::wstring& lastScanTime);
    static bool GetDeferUpgrade(bool& deferUpgrade);

    // Policy CSP
    static bool GetActiveHoursStart(unsigned int& hours);
    static bool SetActiveHoursStart(unsigned int hours);

    static bool GetActiveHoursEnd(unsigned int& hours);
    static bool SetActiveHoursEnd(unsigned int hours);

    static bool GetAllowAutoUpdate(unsigned int& allowLevel);
    static bool SetAllowAutoUpdate(unsigned int allowLevel);

    static bool GetAllowMUUpdateService(unsigned int& allowLevel);
    static bool SetAllowMUUpdateService(unsigned int allowLevel);

    static bool GetAllowNonMicrosoftSignedUpdate(unsigned int& allowLevel);
    static bool SetAllowNonMicrosoftSignedUpdate(unsigned int allowLevel);

    static bool GetAllowUpdateService(unsigned int& allowLevel);
    static bool SetAllowUpdateService(unsigned int allowLevel);

    static bool GetBranchReadinessLevel(unsigned int& level);
    static bool SetBranchReadinessLevel(unsigned int level);

    static bool GetDeferFeatureUpdatesPeriodInDays(unsigned int& days);
    static bool SetDeferFeatureUpdatesPeriodInDays(unsigned int days);

    static bool GetDeferQualityUpdatesPeriodInDays(unsigned int& days);
    static bool SetDeferQualityUpdatesPeriodInDays(unsigned int days);

    static bool GetDeferUpdatePeriod(unsigned int& weeks);
    static bool SetDeferUpdatePeriod(unsigned int weeks);

    static bool GetDeferUpgradePeriod(unsigned int& months);
    static bool SetDeferUpgradePeriod(unsigned int months);

    static bool GetExcludeWUDrivers(unsigned int& excludeLevel);
    static bool SetExcludeWUDrivers(unsigned int excludeLevel);

    static bool GetPauseDeferrals(unsigned int& pauseLevel);
    static bool SetPauseDeferrals(unsigned int pauseLevel);

    static bool GetPauseFeatureUpdates(unsigned int& pauseLevel);
    static bool SetPauseFeatureUpdates(unsigned int pauseLevel);

    static bool GetPauseQualityUpdates(unsigned int& pauseLevel);
    static bool SetPauseQualityUpdates(unsigned int pauseLevel);

    static bool GetRequireDeferUpgrade(unsigned int& value);
    static bool SetRequireDeferUpgrade(unsigned int value);

    static bool GetRequireUpdateApproval(unsigned int& value);
    static bool SetRequireUpdateApproval(unsigned int value);

    static bool GetScheduledInstallDay(unsigned int& dayOfWeek);
    static bool SetScheduledInstallDay(unsigned int dayOfWeek);

    static bool GetScheduledInstallTime(unsigned int& hour);
    static bool SetScheduledInstallTime(unsigned int hour);

    static bool GetUpdateServiceUrl(std::wstring& serviceUrl);
    static bool SetUpdateServiceUrl(const std::wstring& serviceUrl);
};