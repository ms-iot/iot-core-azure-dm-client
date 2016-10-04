#pragma once

#include <string>
#include "MdmProvision.h"

class PolicyCSP
{
public:
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