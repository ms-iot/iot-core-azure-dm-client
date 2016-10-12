#include "stdafx.h"
#include "RebootCSP.h"
#include "MdmProvision.h"

using namespace std;

// Reboot CSP docs
// https://msdn.microsoft.com/en-us/library/windows/hardware/mt720802(v=vs.85).aspx
//

void RebootCSP::ExecRebootNow()
{
    TRACE(L"\n---- Run Reboot Now\n");
    MdmProvision::RunExec(L"./Device/Vendor/MSFT/Reboot/RebootNow");
}

wstring RebootCSP::GetSingleScheduleTime()
{
    TRACE(L"\n---- Get Single Schedule Time\n");
    wstring time = MdmProvision::RunGetString(L"./Device/Vendor/MSFT/Reboot/Schedule/Single");
    TRACEP(L"    :", time.c_str());
    return time;
}

void RebootCSP::SetSingleScheduleTime(const wstring& dailyScheduleTime)
{
    TRACE(L"\n---- Set Single Schedule Time\n");
    MdmProvision::RunSet(L"./Device/Vendor/MSFT/Reboot/Schedule/Single", dailyScheduleTime);
    TRACEP(L"    :", dailyScheduleTime.c_str());
}

wstring RebootCSP::GetDailyScheduleTime()
{
    TRACE(L"\n---- Get Daily Schedule Time\n");
    wstring time = MdmProvision::RunGetString(L"./Device/Vendor/MSFT/Reboot/Schedule/DailyRecurrent");
    TRACEP(L"    :", time.c_str());
    return time;
}

void RebootCSP::SetDailyScheduleTime(const wstring& dailyScheduleTime)
{
    TRACE(L"\n---- Set Daily Schedule Time\n");
    MdmProvision::RunSet(L"./Device/Vendor/MSFT/Reboot/Schedule/DailyRecurrent", dailyScheduleTime);
    TRACEP(L"    :", dailyScheduleTime.c_str());
}
