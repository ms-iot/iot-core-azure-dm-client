#include "stdafx.h"
#include "PolicyCSP.h"
#include "MdmProvision.h"

using namespace std;

// Policy CSP docs
// https://msdn.microsoft.com/en-us/library/windows/hardware/dn904962(v=vs.85).aspx
//

unsigned int PolicyCSP::GetActiveHoursStart()
{
    TRACE(L"\n---- Get Active Hours Start\n");
    unsigned int hours = MdmProvision::RunGetUInt(L"./Device/Vendor/MSFT/Policy/Result/Update/ActiveHoursStart");
    TRACEP(L"    :", hours);
    return hours;
}

// Hours valid range: 0-23
void PolicyCSP::SetActiveHoursStart(unsigned int hours)
{
    TRACE(L"\n---- Set Active Hours Start\n");
    MdmProvision::RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/ActiveHoursStart", hours);
    TRACEP(L"    :", hours);
}

unsigned int PolicyCSP::GetActiveHoursEnd()
{
    TRACE(L"\n---- Get Active Hours End\n");
    unsigned int hours = MdmProvision::RunGetUInt(L"./Device/Vendor/MSFT/Policy/Result/Update/ActiveHoursEnd");
    TRACEP(L"    :", hours);
    return hours;
}

// Hours valid range: 0-23
void PolicyCSP::SetActiveHoursEnd(unsigned int hours)
{
    TRACE(L"\n---- Set Active Hours End\n");
    MdmProvision::RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/ActiveHoursEnd", hours);
    TRACEP(L"    :", hours);
}

unsigned int PolicyCSP::GetAllowAutoUpdate()
{
    TRACE(L"\n---- Get Allow Auto Update\n");
    unsigned int allowLevel = MdmProvision::RunGetUInt(L"./Device/Vendor/MSFT/Policy/Result/Update/AllowAutoUpdate");
    TRACEP(L"    :", allowLevel);
    return allowLevel;
}

// Valid levels: 0-5
void PolicyCSP::SetAllowAutoUpdate(unsigned int allowLevel)
{
    TRACE(L"\n---- Set Allow Auto Update\n");
    MdmProvision::RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/AllowAutoUpdate", allowLevel);
    TRACEP(L"    :", allowLevel);
}

unsigned int PolicyCSP::GetAllowMUUpdateService()
{
    TRACE(L"\n---- Get Allow MU Update Service\n");
    unsigned int allowLevel = MdmProvision::RunGetUInt(L"./Device/Vendor/MSFT/Policy/Result/Update/AllowMUUpdateService");
    TRACEP(L"    :", allowLevel);
    return allowLevel;
}

// Valid levels: 0-1
void PolicyCSP::SetAllowMUUpdateService(unsigned int allowLevel)
{
    TRACE(L"\n---- Set Allow MU Update Service\n");
    MdmProvision::RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/AllowMUUpdateService", allowLevel);
    TRACEP(L"    :", allowLevel);
}

unsigned int PolicyCSP::GetAllowNonMicrosoftSignedUpdate()
{
    TRACE(L"\n---- Allow Allow Non-Microsoft Signed Update\n");
    unsigned int allowLevel = MdmProvision::RunGetUInt(L"./Device/Vendor/MSFT/Policy/Result/Update/AllowNonMicrosoftSignedUpdate");
    TRACEP(L"    :", allowLevel);
    return allowLevel;
}

// Valid levels: 0-1
void PolicyCSP::SetAllowNonMicrosoftSignedUpdate(unsigned int allowLevel)
{
    TRACE(L"\n---- Set Allow Non-Microsoft Signed Update\n");
    MdmProvision::RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/AllowNonMicrosoftSignedUpdate", allowLevel);
    TRACEP(L"    :", allowLevel);
}

unsigned int PolicyCSP::GetAllowUpdateService()
{
    TRACE(L"\n---- Get Allow Update Service\n");
    unsigned int allowLevel = MdmProvision::RunGetUInt(L"./Device/Vendor/MSFT/Policy/Result/Update/AllowUpdateService");
    TRACEP(L"    :", allowLevel);
    return allowLevel;
}

// Valid levels: 0-1
void PolicyCSP::SetAllowUpdateService(unsigned int allowLevel)
{
    TRACE(L"\n---- Set Allow Update Service\n");
    MdmProvision::RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/AllowUpdateService", allowLevel);
    TRACEP(L"    :", allowLevel);
}

unsigned int PolicyCSP::GetBranchReadinessLevel()
{
    TRACE(L"\n---- Get Branch Readiness Level\n");
    unsigned int level = MdmProvision::RunGetUInt(L"./Device/Vendor/MSFT/Policy/Result/Update/BranchReadinessLevel");
    TRACEP(L"    :", level);
    return level;
}

// Valid levels: 0-1
void PolicyCSP::SetBranchReadinessLevel(unsigned int level)
{
    TRACE(L"\n---- Set Branch Readiness Level\n");
    MdmProvision::RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/BranchReadinessLevel", level);
    TRACEP(L"    :", level);
}

unsigned int PolicyCSP::GetDeferFeatureUpdatesPeriodInDays()
{
    TRACE(L"\n---- Get Defer Feature Updates Period In Days\n");
    unsigned int days = MdmProvision::RunGetUInt(L"./Device/Vendor/MSFT/Policy/Result/Update/DeferFeatureUpdatesPeriodInDays");
    TRACEP(L"    :", days);
    return days;
}

// Valid values: 0-180
void PolicyCSP::SetDeferFeatureUpdatesPeriodInDays(unsigned int days)
{
    TRACE(L"\n---- Set Defer Feature Updates Period In Days\n");
    MdmProvision::RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/DeferFeatureUpdatesPeriodInDays", days);
    TRACEP(L"    :", days);
}

unsigned int PolicyCSP::GetDeferQualityUpdatesPeriodInDays()
{
    TRACE(L"\n---- Get Defer Quality Updates Period In Days\n");
    unsigned int days = MdmProvision::RunGetUInt(L"./Device/Vendor/MSFT/Policy/Result/Update/DeferQualityUpdatesPeriodInDays");
    TRACEP(L"    :", days);
    return days;
}

// Valid values: 0-30
void PolicyCSP::SetDeferQualityUpdatesPeriodInDays(unsigned int days)
{
    TRACE(L"\n---- Set Defer Quality Updates Period In Days\n");
    MdmProvision::RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/DeferQualityUpdatesPeriodInDays", days);
    TRACEP(L"    :", days);
}

unsigned int PolicyCSP::GetExcludeWUDrivers()
{
    TRACE(L"\n---- Get ExcludeWUDrivers\n");
    unsigned int excludeLevel = MdmProvision::RunGetUInt(L"./Device/Vendor/MSFT/Policy/Result/Update/ExcludeWUDrivers");
    TRACEP(L"    :", excludeLevel);
    return excludeLevel;
}

// Valid values: 0-1
void PolicyCSP::SetExcludeWUDrivers(unsigned int excludeLevel)
{
    TRACE(L"\n---- Set ExcludeWUDrivers\n");
    MdmProvision::RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/ExcludeWUDrivers", excludeLevel);
    TRACEP(L"    :", excludeLevel);
}

unsigned int PolicyCSP::GetPauseFeatureUpdates()
{
    TRACE(L"\n---- Get PauseFeatureUpdates\n");
    unsigned int pauseLevel = MdmProvision::RunGetUInt(L"./Device/Vendor/MSFT/Policy/Result/Update/PauseFeatureUpdates");
    TRACEP(L"    :", pauseLevel);
    return pauseLevel;
}

// Valid values: 0-1
void PolicyCSP::SetPauseFeatureUpdates(unsigned int pauseLevel)
{
    TRACE(L"\n---- Set PauseFeatureUpdates\n");
    MdmProvision::RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/PauseFeatureUpdates", pauseLevel);
    TRACEP(L"    :", pauseLevel);
}

unsigned int PolicyCSP::GetPauseQualityUpdates()
{
    TRACE(L"\n---- Get PauseQualityUpdates\n");
    unsigned int pauseLevel = MdmProvision::RunGetUInt(L"./Device/Vendor/MSFT/Policy/Result/Update/PauseQualityUpdates");
    TRACEP(L"    :", pauseLevel);
    return pauseLevel;
}

// Valid values: 0-1
void PolicyCSP::SetPauseQualityUpdates(unsigned int pauseLevel)
{
    TRACE(L"\n---- Set PauseQualityUpdates\n");
    MdmProvision::RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/PauseQualityUpdates", pauseLevel);
    TRACEP(L"    :", pauseLevel);
}

unsigned int PolicyCSP::GetRequireUpdateApproval()
{
    TRACE(L"\n---- Get RequireUpdateApproval\n");
    unsigned int value = MdmProvision::RunGetUInt(L"./Device/Vendor/MSFT/Policy/Result/Update/RequireUpdateApproval");
    TRACEP(L"    :", value);
    return value;
}

// Valid values: 0-1
void PolicyCSP::SetRequireUpdateApproval(unsigned int value)
{
    TRACE(L"\n---- Set RequireUpdateApproval\n");
    MdmProvision::RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/RequireUpdateApproval", value);
    TRACEP(L"    :", value);
}

unsigned int PolicyCSP::GetScheduledInstallDay()
{
    TRACE(L"\n---- Get ScheduledInstallDay\n");
    unsigned int dayOfWeek = MdmProvision::RunGetUInt(L"./Device/Vendor/MSFT/Policy/Result/Update/ScheduledInstallDay");
    TRACEP(L"    :", dayOfWeek);
    return dayOfWeek;
}

// Valid values: 0-7
void PolicyCSP::SetScheduledInstallDay(unsigned int dayOfWeek)
{
    TRACE(L"\n---- Set Scheduled Install Day\n");
    MdmProvision::RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/ScheduledInstallDay", dayOfWeek);
    TRACEP(L"    :", dayOfWeek);
}

unsigned int PolicyCSP::GetScheduledInstallTime()
{
    TRACE(L"\n---- Get Scheduled Install Time\n");
    unsigned int hour = MdmProvision::RunGetUInt(L"./Device/Vendor/MSFT/Policy/Result/Update/ScheduledInstallTime");
    TRACEP(L"    :", hour);
    return hour;
}

// Valid values: 0-23
void PolicyCSP::SetScheduledInstallTime(unsigned int hour)
{
    TRACE(L"\n---- Set Scheduled Install Time\n");
    MdmProvision::RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/ScheduledInstallTime", hour);
    TRACEP(L"    :", hour);
}

wstring PolicyCSP::GetUpdateServiceUrl()
{
    TRACE(L"\n---- Get UpdateServiceUrl\n");
    wstring serviceUrl = MdmProvision::RunGetString(L"./Device/Vendor/MSFT/Policy/Result/Update/UpdateServiceUrl");
    TRACEP(L"    ", serviceUrl.c_str());
    return serviceUrl;
}

void PolicyCSP::SetUpdateServiceUrl(const wstring& serviceUrl)
{
    TRACE(L"\n---- Set UpdateServiceUrl\n");
    wstring valueString = serviceUrl;
    MdmProvision::RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/UpdateServiceUrl", valueString);
    TRACEP(L"    ", valueString.c_str());
}

