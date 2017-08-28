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
#include <vector>
#include <map>
#include "TimeCfg.h"
#include "..\SharedUtilities\Utils.h"
#include "..\SharedUtilities\TimeHelpers.h"
#include "..\SharedUtilities\JsonHelpers.h"
#include "..\SharedUtilities\Logger.h"
#include "..\SharedUtilities\DMException.h"
#include "..\SharedUtilities\PolicyHelper.h"
#include "CSPs\MdmProvision.h"
#include "ServiceManager.h"
#include "..\DMShared\ErrorCodes.h"

#include "Models\TimeInfo.h"

#define NtpServerPropertyName "NtpServer"
#define TimeServiceName L"w32time"
#define JsonNo L"no"
#define JsonYes L"yes"
#define JsonNA L"n/a"
#define JsonAuto L"auto"
#define JsonManual L"manual"
#define JsonUnexpected L"unexpected"

using namespace Windows::System;
using namespace Platform;
using namespace Windows::Data::Json;
using namespace Windows::System::Profile;
using namespace Windows::Foundation::Collections;

using namespace Microsoft::Devices::Management::Message;

using namespace std;
using namespace Utils;

#ifdef GetObject
#undef GetObject
#endif

void TimeCfg::Get(TimeInfo& info)
{
    TRACE(__FUNCTION__);

    unsigned long returnCode;
    std::string output;
    Utils::LaunchProcess(L"C:\\windows\\system32\\w32tm.exe /query /configuration", returnCode, output);
    if (returnCode != 0)
    {
        throw DMExceptionWithErrorCode("Error: w32tm.exe returned an error code.", returnCode);
    }

    vector<string> lines;
    Utils::SplitString(output, '\n', lines);
    for (const string& line : lines)
    {
        TRACEP("Line: ", line.c_str());

        vector<string> tokens;
        Utils::SplitString(line, ':', tokens);
        if (tokens.size() == 2)
        {
            string name = Utils::TrimString(tokens[0], string(" "));
            string value = Utils::TrimString(tokens[1], string(" "));

            // remove the trailing " (Local)".
            size_t pos = value.find(" (Local)");
            if (pos != -1)
            {
                value = value.replace(pos, value.length() - pos, "");
            }

            TRACEP("--> Found: ", (name + " = " + value).c_str());
            if (name == NtpServerPropertyName)
            {
                info.ntpServer = MultibyteToWide(value.c_str());
            }
        }
    }

    // Local time
    info.localTime = MdmProvision::RunGetString(L"./DevDetail/Ext/Microsoft/LocalTime");

    // Time zone info...
    TIME_ZONE_INFORMATION tzi = { 0 };
    GetTimeZoneInformation(&info.timeZoneInformation);
}

void TimeCfg::Set(SetTimeInfoRequest^ setTimeInfoRequest)
{
    TRACE(__FUNCTION__);

    SetTimeInfoRequestData^ data = setTimeInfoRequest->data;

    SetNtpServer(data->ntpServer->Data());

    TIME_ZONE_INFORMATION tzi = { 0 };

    // Bias...
    tzi.Bias = data->timeZoneBias;

    TRACEP("Bias: ", to_string(data->timeZoneBias).c_str());

    TRACEP("Standard Bias: ", to_string(data->timeZoneStandardBias).c_str());
    TRACEP(L"Standard Name: ", data->timeZoneStandardName->Data());
    TRACEP(L"Standard Date: ", data->timeZoneStandardDate->Data());
    TRACEP(L"Standard Day of Week: ", to_string(data->timeZoneStandardDayOfWeek).c_str());

    TRACEP("Daytime Bias: ", to_string(data->timeZoneDaylightBias).c_str());
    TRACEP(L"Daytime Name: ", data->timeZoneDaylightName->Data());
    TRACEP(L"Daytime Date: ", data->timeZoneDaylightDate->Data());
    TRACEP(L"Daytime Day of Week: ", to_string(data->timeZoneDaylightDayOfWeek).c_str());

    // Standard...
    wcsncpy_s(tzi.StandardName, data->timeZoneStandardName->Data(), _TRUNCATE);
    if (data->timeZoneStandardDate->Length() == 0)
    {
        // No support for daylight saving time.
        tzi.StandardDate.wMonth = 0;
    }
    else
    {
        if (!SystemTimeFromISO8601(data->timeZoneStandardDate->Data(), tzi.StandardDate))
        {
            throw DMExceptionWithErrorCode("Error: invalid date/time format.", GetLastError());
        }
    }
    tzi.StandardDate.wYear = 0;
    tzi.StandardDate.wDayOfWeek = static_cast<WORD>(data->timeZoneStandardDayOfWeek);
    tzi.StandardBias = data->timeZoneStandardBias;

    // Daytime...
    wcsncpy_s(tzi.DaylightName, data->timeZoneDaylightName->Data(), _TRUNCATE);
    if (data->timeZoneDaylightDate->Length() == 0)
    {
        // No support for daylight saving time.
        tzi.DaylightDate.wMonth = 0;
    }
    else
    {
        if (!SystemTimeFromISO8601(data->timeZoneDaylightDate->Data(), tzi.DaylightDate))
        {
            throw DMExceptionWithErrorCode("Error: invalid date/time format.", GetLastError());
        }
    }
    tzi.DaylightDate.wYear = 0;
    tzi.DaylightDate.wDayOfWeek = static_cast<WORD>(data->timeZoneDaylightDayOfWeek);
    tzi.DaylightBias = data->timeZoneDaylightBias;

    // Set it...
    if (!SetTimeZoneInformation(&tzi))
    {
        throw DMExceptionWithErrorCode("Error: failed to set time zone information.", GetLastError());
    }

    TRACE(L"Time settings have been applied successfully.");
}

void TimeCfg::SetNtpServer(const std::wstring& ntpServer)
{
    TRACE(__FUNCTION__);
    TRACEP(L"New NTP Server = ", ntpServer.c_str());

    wstring command = L"";
    command += L"w32tm /config /manualpeerlist:";
    command += ntpServer;
    command += L" /syncfromflags:manual /reliable:yes /update";

    unsigned long returnCode = 0;
    std::string output;
    Utils::LaunchProcess(command, returnCode, output);
    if (returnCode != 0)
    {
        throw DMExceptionWithErrorCode("Error: w32tm.exe returned an error.", returnCode);
    }
}

GetTimeInfoResponse^ TimeCfg::Get()
{
    TRACE(__FUNCTION__);

    TimeInfo info;
    Get(info);

    GetTimeInfoResponseData^ data = ref new GetTimeInfoResponseData();
    data->localTime = ref new String(info.localTime.c_str());
    data->ntpServer = ref new String(info.ntpServer.c_str());
    data->timeZoneBias = info.timeZoneInformation.Bias;

    data->timeZoneStandardName = ref new String(info.timeZoneInformation.StandardName);
    data->timeZoneStandardDate = ref new String(Utils::ISO8601FromSystemTime(info.timeZoneInformation.StandardDate).c_str());
    data->timeZoneStandardBias = info.timeZoneInformation.StandardBias;
    data->timeZoneStandardDayOfWeek = info.timeZoneInformation.StandardDate.wDayOfWeek;

    data->timeZoneDaylightName = ref new String(info.timeZoneInformation.DaylightName);
    data->timeZoneDaylightDate = ref new String(Utils::ISO8601FromSystemTime(info.timeZoneInformation.DaylightDate).c_str());
    data->timeZoneDaylightBias = info.timeZoneInformation.DaylightBias;
    data->timeZoneDaylightDayOfWeek = info.timeZoneInformation.DaylightDate.wDayOfWeek;

    return ref new GetTimeInfoResponse(ResponseStatus::Success, data);
}

TimeServiceData^ TimeService::GetState()
{
    TRACE(__FUNCTION__);

    TimeServiceData^ data = ref new TimeServiceData();

    if (ServiceManager::GetStartType(TimeServiceName) == SERVICE_DISABLED)
    {
        data->enabled = ref new String(JsonNo);
        data->startup = ref new String(JsonNA);
        data->started = ref new String(JsonNA);
    }
    else
    {
        data->enabled = ref new String(JsonYes);
        if (ServiceManager::GetStartType(TimeServiceName) == SERVICE_AUTO_START)
        {
            data->startup = ref new String(JsonAuto);
        }
        else if (ServiceManager::GetStartType(TimeServiceName) == SERVICE_DEMAND_START)
        {
            data->startup = ref new String(JsonManual);
        }
        else
        {
            data->startup = ref new String(JsonUnexpected);
        }

        DWORD status = ServiceManager::GetStatus(TimeServiceName);

        data->started = ref new String((status == SERVICE_RUNNING) ? JsonYes : JsonNo);
    }
    data->policy = PolicyHelper::LoadFromRegistry(RegTimeService);

    return data;
}

void TimeService::SaveState(TimeServiceData^ data)
{
    TRACE(__FUNCTION__);

    if (!data->policy)
    {
        throw DMExceptionWithErrorCode("Policy unspecified while storing Time Service settings", ERROR_DM_TIME_SERVICE_MISSING_POLICY);
    }

    // Save source priorities...
    PolicyHelper::SaveToRegistry(data->policy, RegTimeService);

    // Save remote/local properties...
    if (data->policy->source == PolicySource::Remote)
    {
        TRACE("Writing remote policy...");
        Utils::WriteRegistryValue(RegRemoteTimeService, RegTimeServiceEnabled, data->enabled->Data());
        Utils::WriteRegistryValue(RegRemoteTimeService, RegTimeServiceStartup, data->startup->Data());
        Utils::WriteRegistryValue(RegRemoteTimeService, RegTimeServiceStarted, data->started->Data());
    }
    else
    {
        TRACE("Writing local policy...");
        Utils::WriteRegistryValue(RegLocalTimeService, RegTimeServiceEnabled, data->enabled->Data());
        Utils::WriteRegistryValue(RegLocalTimeService, RegTimeServiceStartup, data->startup->Data());
        Utils::WriteRegistryValue(RegLocalTimeService, RegTimeServiceStarted, data->started->Data());
    }
}

TimeServiceData^ TimeService::GetActiveDesiredState()
{
    TRACE(__FUNCTION__);

    Policy^ policy = PolicyHelper::LoadFromRegistry(RegTimeService);
    if (!policy)
    {
        TRACE("Active desired state for Time Service is not set.");
        return nullptr;
    }

    for each(PolicySource p in policy->sourcePriorities)
    {
        wstring regSectionRoot = L"";
        switch (p)
        {
            case PolicySource::Local:
                TRACE("Reading local policy for Time Service.");
                regSectionRoot = RegLocalTimeService;
                break;
            case PolicySource::Remote:
                TRACE("Reading remote policy for Time Service.");
                regSectionRoot = RegRemoteTimeService;
                break;
        }

        bool success = true;

        wstring enabled;
        success &= ERROR_SUCCESS == Utils::TryReadRegistryValue(regSectionRoot.c_str(), RegTimeServiceEnabled, enabled);

        wstring startup;
        success &= ERROR_SUCCESS == Utils::TryReadRegistryValue(regSectionRoot.c_str(), RegTimeServiceStartup, startup);

        wstring started;
        success &= ERROR_SUCCESS == Utils::TryReadRegistryValue(regSectionRoot.c_str(), RegTimeServiceStarted, started);

        if (!success)
        {
            TRACE("Did not find one or more attributes in the registry.");

            // Try reading the next policy...
            continue;
        }

        TRACE("Found all Time Service attributes in the registry.");

        TimeServiceData^ data = ref new TimeServiceData();
        data->enabled = ref new String(enabled.c_str());
        data->startup = ref new String(startup.c_str());
        data->started = ref new String(started.c_str());
        data->policy = policy;
        return data;
    }

    TRACE("Could not find active desired state for Time Service.");
    return nullptr;
}

void TimeService::SetState(TimeServiceData^ data)
{
    TRACE(__FUNCTION__);

    SaveState(data);
    TimeServiceData^ activeDesiredState = GetActiveDesiredState();

    TRACE("Applying active desired state");
    if (activeDesiredState->enabled == JsonNo)
    {
        ServiceManager::Stop(TimeServiceName);
        ServiceManager::SetStartType(TimeServiceName, SERVICE_DISABLED);
    }
    else
    {
        if (activeDesiredState->startup == JsonAuto)
        {
            ServiceManager::SetStartType(TimeServiceName, SERVICE_AUTO_START);
        }
        else if (activeDesiredState->startup == JsonManual)
        {
            ServiceManager::SetStartType(TimeServiceName, SERVICE_DEMAND_START);
        }

        if (activeDesiredState->started == JsonYes)
        {
            ServiceManager::Start(TimeServiceName);
        }
        else
        {
            ServiceManager::Stop(TimeServiceName);
        }
    }
}
