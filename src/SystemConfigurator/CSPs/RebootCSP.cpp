/*
Copyright 2017 Microsoft
Permission is hereby granted, free of charge, to any person obtaining a copy of this software 
and associated documentation files (the "Software"), to deal in the Software without restriction, 
including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, 
subject to the following conditions:

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT 
LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH 
THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#include "stdafx.h"
#include <map>
#include "RebootCSP.h"
#include "MdmProvision.h"
#include "..\SharedUtilities\DMException.h"
#include "..\SharedUtilities\Logger.h"
#include "..\SharedUtilities\JsonHelpers.h"
#include "..\SharedUtilities\TimeHelpers.h"

using namespace std;

// Reboot CSP docs
// https://msdn.microsoft.com/en-us/library/windows/hardware/mt720802(v=vs.85).aspx
//

wstring RebootCSP::_lastRebootTime;

RebootCSP gRebootCSP;

RebootCSP::RebootCSP()
{
    TRACE(__FUNCTION__);

    _lastRebootTime = Utils::GetCurrentDateTimeString();
}

void RebootCSP::ExecRebootNow(const wstring& lastRebootCmdTime)
{
    TRACE(__FUNCTION__);

    Utils::WriteRegistryValue(IoTDMRegistryRoot, IoTDMRegistryLastRebootCmd, lastRebootCmdTime);

    TRACE(L"\n---- Run Reboot Now\n");
    MdmProvision::RunExec(L"./Device/Vendor/MSFT/Reboot/RebootNow");
}

wstring RebootCSP::GetLastRebootCmdTime()
{
    TRACE(__FUNCTION__);

    wstring lastRebootCmdTime = L"";
    try
    {
        Utils::ReadRegistryValue(IoTDMRegistryRoot, IoTDMRegistryLastRebootCmd);
    }
    catch (DMException&)
    {
        // This is okay to ignore since this device might have never received a reboot command through DM.
    }
    return lastRebootCmdTime;
}

std::wstring RebootCSP::GetLastRebootTime()
{
    TRACE(__FUNCTION__);

    return _lastRebootTime;
}

wstring RebootCSP::GetSingleScheduleTime()
{
    TRACE(L"\n---- Get Single Schedule Time\n");
    wstring time = MdmProvision::RunGetString(L"./Device/Vendor/MSFT/Reboot/Schedule/Single");
    if (time.size())
    {
        time = Utils::CanonicalizeDateTime(time);   // CSP sometimes returns 2016-10-10T09:00:01-008:00, the 008 breaks .net parsing.
    }
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
    if (time.size())
    {
        time = Utils::CanonicalizeDateTime(time);   // CSP sometimes returns 2016-10-10T09:00:01-008:00, the 008 breaks .net parsing.
    }
    TRACEP(L"    :", time.c_str());
    return time;
}

void RebootCSP::SetDailyScheduleTime(const wstring& dailyScheduleTime)
{
    TRACE(L"\n---- Set Daily Schedule Time\n");
    MdmProvision::RunSet(L"./Device/Vendor/MSFT/Reboot/Schedule/DailyRecurrent", dailyScheduleTime);
    TRACEP(L"    :", dailyScheduleTime.c_str());
}
