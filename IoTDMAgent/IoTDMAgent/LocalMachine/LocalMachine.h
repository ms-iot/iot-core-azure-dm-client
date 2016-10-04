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
    static void RunSyncML(const std::wstring& request, std::wstring& response);
    static void RunSyncMLAdd(const std::wstring& path, const std::wstring& value);
    static std::wstring RunSyncMLGetString(const std::wstring& path);
    static unsigned int RunSyncMLGetInt(const std::wstring& path);
    static void RunSyncMLSet(const std::wstring& path, const std::wstring& value);
    static void RunSyncMLSet(const std::wstring& path, unsigned int value);

    // Update CSP
    static void GetInstalledUpdates(std::vector<std::wstring>& guids);
    static void GetApprovedUpdates(std::vector<std::wstring>& guids);
    static void AddApprovedUpdates(const std::wstring& guid);
    static void GetFailedUpdates(std::vector<std::wstring>& guids);
    static void GetInstallableUpdates(std::vector<std::wstring>& guids);
    static void GetPendingRebootUpdates(std::vector<std::wstring>& guids);
    static std::wstring GetLastSuccessfulScanTime();
    static bool GetDeferUpgrade();

    // Policy CSP
    static unsigned int GetActiveHoursStart();
    static void SetActiveHoursStart(unsigned int hours);

    static unsigned int GetActiveHoursEnd();
    static void SetActiveHoursEnd(unsigned int hours);

    static unsigned int GetAllowAutoUpdate();
    static void SetAllowAutoUpdate(unsigned int allowLevel);

    static unsigned int GetAllowMUUpdateService();
    static void SetAllowMUUpdateService(unsigned int allowLevel);

    static unsigned int GetAllowNonMicrosoftSignedUpdate();
    static void SetAllowNonMicrosoftSignedUpdate(unsigned int allowLevel);

    static unsigned int GetAllowUpdateService();
    static void SetAllowUpdateService(unsigned int allowLevel);

    static unsigned int GetBranchReadinessLevel();
    static void SetBranchReadinessLevel(unsigned int level);

    static unsigned int GetDeferFeatureUpdatesPeriodInDays();
    static void SetDeferFeatureUpdatesPeriodInDays(unsigned int days);

    static unsigned int GetDeferQualityUpdatesPeriodInDays();
    static void SetDeferQualityUpdatesPeriodInDays(unsigned int days);

    static unsigned int GetDeferUpdatePeriod();
    static void SetDeferUpdatePeriod(unsigned int weeks);

    static unsigned int GetDeferUpgradePeriod();
    static void SetDeferUpgradePeriod(unsigned int months);

    static unsigned int GetExcludeWUDrivers();
    static void SetExcludeWUDrivers(unsigned int excludeLevel);

    static unsigned int GetPauseDeferrals();
    static void SetPauseDeferrals(unsigned int pauseLevel);

    static unsigned int GetPauseFeatureUpdates();
    static void SetPauseFeatureUpdates(unsigned int pauseLevel);

    static unsigned int GetPauseQualityUpdates();
    static void SetPauseQualityUpdates(unsigned int pauseLevel);

    static unsigned int GetRequireDeferUpgrade();
    static void SetRequireDeferUpgrade(unsigned int value);

    static unsigned int GetRequireUpdateApproval();
    static void SetRequireUpdateApproval(unsigned int value);

    static unsigned int GetScheduledInstallDay();
    static void SetScheduledInstallDay(unsigned int dayOfWeek);

    static unsigned int GetScheduledInstallTime();
    static void SetScheduledInstallTime(unsigned int hour);

    static std::wstring GetUpdateServiceUrl();
    static void SetUpdateServiceUrl(const std::wstring& serviceUrl);
};