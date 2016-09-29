#include "stdafx.h"
#include "PolicyCSP.h"
#include "..\..\Utilities\Utils.h"
#include "..\..\Utilities\Logger.h"

using namespace std;

// Policy CSP docs
// https://msdn.microsoft.com/en-us/library/windows/hardware/dn904962(v=vs.85).aspx
//

bool PolicyCSP::GetActiveHoursStart(unsigned int& hours)
{
    TRACE(L"\n---- Get Active Hours Start\n");
    if (RunGet(L"./Device/Vendor/MSFT/Policy/Result/Update/ActiveHoursStart", hours))
    {
        TRACEP(L"    :", hours);
        return true;
    }
    return false;
}

// Hours valid range: 0-23
bool PolicyCSP::SetActiveHoursStart(unsigned int hours)
{
    TRACE(L"\n---- Set Active Hours Start\n");
    if (hours > 23)
    {
        TRACE(L"Error: parameter out of range!");
        return false;
    }

    if (RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/ActiveHoursStart", hours))
    {
        TRACEP(L"    :", hours);
        return true;
    }
    return false;
}

bool PolicyCSP::GetActiveHoursEnd(unsigned int& hours)
{
    TRACE(L"\n---- Get Active Hours End\n");
    if (RunGet(L"./Device/Vendor/MSFT/Policy/Result/Update/ActiveHoursEnd", hours))
    {
        TRACEP(L"    :", hours);
        return true;
    }
    return false;
}

// Hours valid range: 0-23
bool PolicyCSP::SetActiveHoursEnd(unsigned int hours)
{
    TRACE(L"\n---- Set Active Hours End\n");
    if (hours > 23)
    {
        TRACE(L"Error: parameter out of range!");
        return false;
    }

    if (RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/ActiveHoursEnd", hours))
    {
        TRACEP(L"    :", hours);
        return true;
    }
    return false;
}

bool PolicyCSP::GetAllowAutoUpdate(unsigned int& allowLevel)
{
    TRACE(L"\n---- Get Allow Auto Update\n");
    if (RunGet(L"./Device/Vendor/MSFT/Policy/Result/Update/AllowAutoUpdate", allowLevel))
    {
        TRACEP(L"    :", allowLevel);
        return true;
    }
    return false;
}

// Valid levels: 0-5
bool PolicyCSP::SetAllowAutoUpdate(unsigned int allowLevel)
{
    TRACE(L"\n---- Set Allow Auto Update\n");
    if (allowLevel > 5)
    {
        TRACE(L"Error: parameter out of range!");
        return false;
    }

    if (RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/AllowAutoUpdate", allowLevel))
    {
        TRACEP(L"    :", allowLevel);
        return true;
    }
    return false;
}

bool PolicyCSP::GetAllowMUUpdateService(unsigned int& allowLevel)
{
    TRACE(L"\n---- Get Allow MU Update Service\n");
    if (RunGet(L"./Device/Vendor/MSFT/Policy/Result/Update/AllowMUUpdateService", allowLevel))
    {
        TRACEP(L"    :", allowLevel);
        return true;
    }
    return false;
}

// Valid levels: 0-1
bool PolicyCSP::SetAllowMUUpdateService(unsigned int allowLevel)
{
    TRACE(L"\n---- Set Allow MU Update Service\n");
    if (allowLevel > 1)
    {
        TRACE(L"Error: parameter out of range!");
        return false;
    }

    if (RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/AllowMUUpdateService", allowLevel))
    {
        TRACEP(L"    :", allowLevel);
        return true;
    }
    return false;
}

bool PolicyCSP::GetAllowNonMicrosoftSignedUpdate(unsigned int& allowLevel)
{
    TRACE(L"\n---- Allow Allow Non-Microsoft Signed Update\n");
    if (RunGet(L"./Device/Vendor/MSFT/Policy/Result/Update/AllowNonMicrosoftSignedUpdate", allowLevel))
    {
        TRACEP(L"    :", allowLevel);
        return true;
    }
    return false;
}

// Valid levels: 0-1
bool PolicyCSP::SetAllowNonMicrosoftSignedUpdate(unsigned int allowLevel)
{
    TRACE(L"\n---- Set Allow Non-Microsoft Signed Update\n");
    if (allowLevel > 1)
    {
        TRACE(L"Error: parameter out of range!");
        return false;
    }

    if (RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/AllowNonMicrosoftSignedUpdate", allowLevel))
    {
        TRACEP(L"    :", allowLevel);
        return true;
    }
    return false;
}

bool PolicyCSP::GetAllowUpdateService(unsigned int& allowLevel)
{
    TRACE(L"\n---- Get Allow Update Service\n");
    if (RunGet(L"./Device/Vendor/MSFT/Policy/Result/Update/AllowUpdateService", allowLevel))
    {
        TRACEP(L"    :", allowLevel);
        return true;
    }
    return false;
}

// Valid levels: 0-1
bool PolicyCSP::SetAllowUpdateService(unsigned int allowLevel)
{
    TRACE(L"\n---- Set Allow Update Service\n");
    if (allowLevel > 1)
    {
        TRACE(L"Error: parameter out of range!");
        return false;
    }

    if (RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/AllowUpdateService", allowLevel))
    {
        TRACEP(L"    :", allowLevel);
        return true;
    }
    return false;
}

bool PolicyCSP::GetBranchReadinessLevel(unsigned int& level)
{
    TRACE(L"\n---- Get Branch Readiness Level\n");
    if (RunGet(L"./Device/Vendor/MSFT/Policy/Result/Update/BranchReadinessLevel", level))
    {
        TRACEP(L"    :", level);
        return true;
    }
    return false;
}

// Valid levels: 0-1
bool PolicyCSP::SetBranchReadinessLevel(unsigned int level)
{
    TRACE(L"\n---- Set Branch Readiness Level\n");
    if (level > 1)
    {
        TRACE(L"Error: parameter out of range!");
        return false;
    }

    if (RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/BranchReadinessLevel", level))
    {
        TRACEP(L"    :", level);
        return true;
    }
    return false;
}

bool PolicyCSP::GetDeferFeatureUpdatesPeriodInDays(unsigned int& days)
{
    TRACE(L"\n---- Get Defer Feature Updates Period In Days\n");
    if (RunGet(L"./Device/Vendor/MSFT/Policy/Result/Update/DeferFeatureUpdatesPeriodInDays", days))
    {
        TRACEP(L"    :", days);
        return true;
    }
    return false;
}

// Valid values: 0-180
bool PolicyCSP::SetDeferFeatureUpdatesPeriodInDays(unsigned int days)
{
    TRACE(L"\n---- Set Defer Feature Updates Period In Days\n");
    if (days > 180)
    {
        TRACE(L"Error: parameter out of range!");
        return false;
    }

    if (RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/DeferFeatureUpdatesPeriodInDays", days))
    {
        TRACEP(L"    :", days);
        return true;
    }
    return false;
}

bool PolicyCSP::GetDeferQualityUpdatesPeriodInDays(unsigned int& days)
{
    TRACE(L"\n---- Get Defer Quality Updates Period In Days\n");
    if (RunGet(L"./Device/Vendor/MSFT/Policy/Result/Update/DeferQualityUpdatesPeriodInDays", days))
    {
        TRACEP(L"    :", days);
        return true;
    }
    return false;
}

// Valid values: 0-30
bool PolicyCSP::SetDeferQualityUpdatesPeriodInDays(unsigned int days)
{
    TRACE(L"\n---- Set Defer Quality Updates Period In Days\n");
    if (days > 30)
    {
        TRACE(L"Error: parameter out of range!");
        return false;
    }

    if (RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/DeferQualityUpdatesPeriodInDays", days))
    {
        TRACEP(L"    :", days);
        return true;
    }
    return false;
}

bool PolicyCSP::GetDeferUpdatePeriod(unsigned int& weeks)
{
    TRACE(L"\n---- Get Defer Update Period\n");
    if (RunGet(L"./Device/Vendor/MSFT/Policy/Result/Update/DeferUpdatePeriod", weeks))
    {
        TRACEP(L"    :", weeks);
        return true;
    }
    return false;
}

// Valid values: 0-4
bool PolicyCSP::SetDeferUpdatePeriod(unsigned int weeks)
{
    TRACE(L"\n---- Set Defer Update Period\n");
    if (weeks > 4)
    {
        TRACE(L"Error: parameter out of range!");
        return false;
    }

    if (RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/DeferUpdatePeriod", weeks))
    {
        TRACEP(L"    :", weeks);
        return true;
    }
    return false;
}

bool PolicyCSP::GetDeferUpgradePeriod(unsigned int& months)
{
    TRACE(L"\n---- Get Defer Upgrade Period\n");
    if (RunGet(L"./Device/Vendor/MSFT/Policy/Result/Update/DeferUpgradePeriod", months))
    {
        TRACEP(L"    :", months);
        return true;
    }
    return false;
}

// Valid values: 0-8
bool PolicyCSP::SetDeferUpgradePeriod(unsigned int months)
{
    TRACE(L"\n---- Set Defer Upgrade Period\n");
    if (months > 8)
    {
        TRACE(L"Error: parameter out of range!");
        return false;
    }

    if (RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/DeferUpgradePeriod", months))
    {
        TRACEP(L"    :", months);
        return true;
    }
    return false;
}

bool PolicyCSP::GetExcludeWUDrivers(unsigned int& excludeLevel)
{
    TRACE(L"\n---- Get ExcludeWUDrivers\n");
    if (RunGet(L"./Device/Vendor/MSFT/Policy/Result/Update/ExcludeWUDrivers", excludeLevel))
    {
        TRACEP(L"    :", excludeLevel);
        return true;
    }
    return false;
}

// Valid values: 0-1
bool PolicyCSP::SetExcludeWUDrivers(unsigned int excludeLevel)
{
    TRACE(L"\n---- Set ExcludeWUDrivers\n");
    if (excludeLevel > 1)
    {
        TRACE(L"Error: parameter out of range!");
        return false;
    }

    if (RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/ExcludeWUDrivers", excludeLevel))
    {
        TRACEP(L"    :", excludeLevel);
        return true;
    }
    return false;
}

bool PolicyCSP::GetPauseDeferrals(unsigned int& pauseLevel)
{
    TRACE(L"\n---- Get PauseDeferrals\n");
    if (RunGet(L"./Device/Vendor/MSFT/Policy/Result/Update/PauseDeferrals", pauseLevel))
    {
        TRACEP(L"    :", pauseLevel);
        return true;
    }
    return false;
}

// Valid values: 0-1
bool PolicyCSP::SetPauseDeferrals(unsigned int pauseLevel)
{
    TRACE(L"\n---- Set PauseDeferrals\n");
    if (pauseLevel > 1)
    {
        TRACE(L"Error: parameter out of range!");
        return false;
    }

    if (RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/PauseDeferrals", pauseLevel))
    {
        TRACEP(L"    :", pauseLevel);
        return true;
    }
    return false;
}

bool PolicyCSP::GetPauseFeatureUpdates(unsigned int& pauseLevel)
{
    TRACE(L"\n---- Get PauseFeatureUpdates\n");
    if (RunGet(L"./Device/Vendor/MSFT/Policy/Result/Update/PauseFeatureUpdates", pauseLevel))
    {
        TRACEP(L"    :", pauseLevel);
        return true;
    }
    return false;
}

// Valid values: 0-1
bool PolicyCSP::SetPauseFeatureUpdates(unsigned int pauseLevel)
{
    TRACE(L"\n---- Set PauseFeatureUpdates\n");
    if (pauseLevel > 1)
    {
        TRACE(L"Error: parameter out of range!");
        return false;
    }

    if (RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/PauseFeatureUpdates", pauseLevel))
    {
        TRACEP(L"    :", pauseLevel);
        return true;
    }
    return false;
}

bool PolicyCSP::GetPauseQualityUpdates(unsigned int& pauseLevel)
{
    TRACE(L"\n---- Get PauseQualityUpdates\n");
    if (RunGet(L"./Device/Vendor/MSFT/Policy/Result/Update/PauseQualityUpdates", pauseLevel))
    {
        TRACEP(L"    :", pauseLevel);
        return true;
    }
    return false;
}

// Valid values: 0-1
bool PolicyCSP::SetPauseQualityUpdates(unsigned int pauseLevel)
{
    TRACE(L"\n---- Set PauseQualityUpdates\n");
    if (pauseLevel > 1)
    {
        TRACE(L"Error: parameter out of range!");
        return false;
    }

    if (RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/PauseQualityUpdates", pauseLevel))
    {
        TRACEP(L"    :", pauseLevel);
        return true;
    }
    return false;
}

bool PolicyCSP::GetRequireDeferUpgrade(unsigned int& value)
{
    TRACE(L"\n---- Get RequireDeferUpgrade\n");
    if (RunGet(L"./Device/Vendor/MSFT/Policy/Result/Update/RequireDeferUpgrade", value))
    {
        TRACEP(L"    :", value);
        return true;
    }
    return false;
}

// Valid values: 0-1
bool PolicyCSP::SetRequireDeferUpgrade(unsigned int value)
{
    TRACE(L"\n---- Set RequireDeferUpgrade\n");
    if (value > 1)
    {
        TRACE(L"Error: parameter out of range!");
        return false;
    }

    if (RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/RequireDeferUpgrade", value))
    {
        TRACEP(L"    :", value);
        return true;
    }
    return false;
}

bool PolicyCSP::GetRequireUpdateApproval(unsigned int& value)
{
    TRACE(L"\n---- Get RequireUpdateApproval\n");
    if (RunGet(L"./Device/Vendor/MSFT/Policy/Result/Update/RequireUpdateApproval", value))
    {
        TRACEP(L"    :", value);
        return true;
    }
    return false;
}

// Valid values: 0-1
bool PolicyCSP::SetRequireUpdateApproval(unsigned int value)
{
    TRACE(L"\n---- Set RequireUpdateApproval\n");
    if (value > 1)
    {
        TRACE(L"Error: parameter out of range!");
        return false;
    }

    if (RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/RequireUpdateApproval", value))
    {
        TRACEP(L"    :", value);
        return true;
    }
    return false;
}

bool PolicyCSP::GetScheduledInstallDay(unsigned int& dayOfWeek)
{
    TRACE(L"\n---- Get ScheduledInstallDay\n");
    if (RunGet(L"./Device/Vendor/MSFT/Policy/Result/Update/ScheduledInstallDay", dayOfWeek))
    {
        TRACEP(L"    :", dayOfWeek);
        return true;
    }
    return false;
}

// Valid values: 0-7
bool PolicyCSP::SetScheduledInstallDay(unsigned int dayOfWeek)
{
    TRACE(L"\n---- Set Scheduled Install Day\n");
    if (dayOfWeek > 7)
    {
        TRACE(L"Error: parameter out of range!");
        return false;
    }

    if (RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/ScheduledInstallDay", dayOfWeek))
    {
        TRACEP(L"    :", dayOfWeek);
        return true;
    }
    return false;
}

bool PolicyCSP::GetScheduledInstallTime(unsigned int& hour)
{
    TRACE(L"\n---- Get Scheduled Install Time\n");
    if (RunGet(L"./Device/Vendor/MSFT/Policy/Result/Update/ScheduledInstallTime", hour))
    {
        TRACEP(L"    :", hour);
        return true;
    }
    return false;
}

// Valid values: 0-23
bool PolicyCSP::SetScheduledInstallTime(unsigned int hour)
{
    TRACE(L"\n---- Set Scheduled Install Time\n");
    if (hour > 23)
    {
        TRACE(L"Error: parameter out of range!");
        return false;
    }

    if (RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/ScheduledInstallTime", hour))
    {
        TRACEP(L"    :", hour);
        return true;
    }
    return false;
}

bool PolicyCSP::GetUpdateServiceUrl(std::wstring& serviceUrl)
{
    TRACE(L"\n---- Get UpdateServiceUrl\n");
    if (RunGet(L"./Device/Vendor/MSFT/Policy/Result/Update/UpdateServiceUrl", serviceUrl))
    {
        TRACEP(L"    ", serviceUrl.c_str());
        return true;
    }
    return false;
}

bool PolicyCSP::SetUpdateServiceUrl(const std::wstring& serviceUrl)
{
    TRACE(L"\n---- Set UpdateServiceUrl\n");

    wstring valueString = serviceUrl;
    if (RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/UpdateServiceUrl", valueString))
    {
        TRACEP(L"    ", valueString.c_str());
        return true;
    }
    return false;
}

