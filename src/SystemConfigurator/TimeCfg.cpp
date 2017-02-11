#include "stdafx.h"
#include <vector>
#include <map>
#include "TimeCfg.h"
#include "..\SharedUtilities\Utils.h"
#include "..\SharedUtilities\TimeHelpers.h"
#include "..\SharedUtilities\JsonHelpers.h"
#include "..\SharedUtilities\Logger.h"
#include "..\SharedUtilities\DMException.h"
#include "CSPs\MdmProvision.h"

#include "Models\TimeInfo.h"

#define NtpServerPropertyName "NtpServer"

// Json strings
/*
#define LocalTime L"localTime"
#define NtpServer L"ntpServer"
#define TimeZone L"timeZone"
#define TimeZoneBias L"bias"
#define TimeZoneStandardName L"standardName"
#define TimeZoneStandardDate L"standardDate"
#define TimeZoneStandardBias L"standardBias"
#define TimeZoneDaylightName L"daylightName"
#define TimeZoneDaylightDate L"daylightDate"
#define TimeZoneDaylightBias L"daylightBias"
*/

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

void TimeCfg::GetTimeInfo(TimeInfo& timeInfo)
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
                timeInfo.ntpServer = MultibyteToWide(value.c_str());
            }
        }
    }

    // Local time
    timeInfo.localTime = MdmProvision::RunGetString(L"./DevDetail/Ext/Microsoft/LocalTime");

    // Time zone info...
    TIME_ZONE_INFORMATION tzi = { 0 };
    GetTimeZoneInformation(&timeInfo.timeZoneInformation);
}

void TimeCfg::SetTimeInfo(IRequest^ request)
{
    TRACE(__FUNCTION__);

    auto setTimeInfoRequest = dynamic_cast<SetTimeInfoRequest^>(request);

    SetNtpServer(setTimeInfoRequest->TimeInfo->NtpServer->Data());

    TIME_ZONE_INFORMATION tzi = { 0 };

    // Bias...
    tzi.Bias = setTimeInfoRequest->TimeInfo->TimeZoneBias;

    TRACEP("Bias: ", to_string(setTimeInfoRequest->TimeInfo->TimeZoneBias).c_str());

    TRACEP("Standard Bias: ", to_string(setTimeInfoRequest->TimeInfo->TimeZoneStandardBias).c_str());
    TRACEP(L"Standard Name: ", setTimeInfoRequest->TimeInfo->TimeZoneStandardName->Data());
    TRACEP(L"Standard Date: ", setTimeInfoRequest->TimeInfo->TimeZoneStandardDate->Data());

    TRACEP("Daytime Bias: ", to_string(setTimeInfoRequest->TimeInfo->TimeZoneDaylightBias).c_str());
    TRACEP(L"Daytime Name: ", setTimeInfoRequest->TimeInfo->TimeZoneDaylightName->Data());
    TRACEP(L"Daytime Date: ", setTimeInfoRequest->TimeInfo->TimeZoneDaylightDate->Data());

    // Standard...
    wcsncpy_s(tzi.StandardName, setTimeInfoRequest->TimeInfo->TimeZoneStandardName->Data(), _TRUNCATE);
    if (!SystemTimeFromISO8601(setTimeInfoRequest->TimeInfo->TimeZoneStandardDate->Data(), tzi.StandardDate))
    {
        throw DMExceptionWithErrorCode("Error: invalid date/time format. Error Code = ", GetLastError());
    }
    tzi.StandardBias = setTimeInfoRequest->TimeInfo->TimeZoneStandardBias;

    // Daytime...
    wcsncpy_s(tzi.DaylightName, setTimeInfoRequest->TimeInfo->TimeZoneDaylightName->Data(), _TRUNCATE);
    if (!SystemTimeFromISO8601(setTimeInfoRequest->TimeInfo->TimeZoneDaylightDate->Data(), tzi.DaylightDate))
    {
        throw DMExceptionWithErrorCode("Error: invalid date/time format. Error Code = ", GetLastError());
    }
    tzi.DaylightBias = setTimeInfoRequest->TimeInfo->TimeZoneDaylightBias;

    // Set it...
    if (!SetTimeZoneInformation(&tzi))
    {
        throw DMExceptionWithErrorCode("Error: failed to set time zone information. Error Code = ", GetLastError());
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

TimeInfoResponse^ TimeCfg::GetTimeInfo()
{
    TRACE(__FUNCTION__);
    TimeInfoResponse ^response;
    try
    {
        TimeInfo timeInfo;
        GetTimeInfo(timeInfo);
        response = ref new TimeInfoResponse(ResponseStatus::Success);
        response->LocalTime = ref new String(timeInfo.localTime.c_str());
        response->NtpServer = ref new String(timeInfo.ntpServer.c_str());
        response->TimeZoneBias = timeInfo.timeZoneInformation.Bias;
        response->TimeZoneStandardName = ref new String(timeInfo.timeZoneInformation.StandardName);
        response->TimeZoneStandardDate = ref new String(Utils::ISO8601FromSystemTime(timeInfo.timeZoneInformation.StandardDate).c_str());
        response->TimeZoneStandardBias = timeInfo.timeZoneInformation.StandardBias;
        response->TimeZoneDaylightName = ref new String(timeInfo.timeZoneInformation.DaylightName);
        response->TimeZoneDaylightDate = ref new String(Utils::ISO8601FromSystemTime(timeInfo.timeZoneInformation.DaylightDate).c_str());
        response->TimeZoneDaylightBias = timeInfo.timeZoneInformation.DaylightBias;
    }
    catch (...)
    {
        response = ref new TimeInfoResponse(ResponseStatus::Failure);
    }
    return response;
}
