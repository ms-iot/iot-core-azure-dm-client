#include "stdafx.h"
#include <vector>
#include <map>
#include "TimeCfg.h"
#include "..\SharedUtilities\Utils.h"
#include "..\SharedUtilities\JsonHelpers.h"
#include "..\SharedUtilities\Logger.h"
#include "..\SharedUtilities\DMException.h"
#include "CSPs\MdmProvision.h"

#define NtpServerPropertyName "NtpServer"

// Json strings
#define TimeInfoNodeName L"timeInfo"
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

using namespace Windows::System;
using namespace Platform;
using namespace Windows::Data::Json;
using namespace Windows::System::Profile;
using namespace Windows::Foundation::Collections;

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

void TimeCfg::SetTimeInfo(const std::wstring& jsonString)
{
    TRACE(__FUNCTION__);
    TRACEP(L" json = ", jsonString.c_str());

    /*
    {
        "timeInfo":
        {
            "ntpServer": "pool.ntp.org",
            "timeZone" :
            {
                "bias": 123,
                "standardName": "(UTC-05:00) Eastern Time (US & Canada)",
                "standardDate": "yyyy-mm-ddThh:mm:ss,day_of_week",
                "standardBias": 33,
                "daylightName": "(UTC-05:00) Eastern Time (US & Canada)",
                "daylightDate": "yyyy-mm-ddThh:mm:ss,day_of_week",
                "daylightBias": 33
            }
        }
    }
    */

    JsonValue^ value;
    if (!JsonValue::TryParse(ref new String(jsonString.c_str()), &value) || (value == nullptr))
    {
        throw DMException("Warning: SetTimeInfo(): Invalid json.");
    }

    JsonObject^ rootObject = value->GetObject();
    if (rootObject == nullptr)
    {
        throw DMException("Warning: SetTimeInfo(): Invalid json input. Cannot find the root object.");
    }

    map<wstring, IJsonValue^> properties;
    JsonReader::Flatten(L"", rootObject, properties);

    wstring ntpServer;
    if (JsonReader::TryFindString(properties, TimeInfoNodeName L"." NtpServer, ntpServer))
    {
        SetNtpServer(ntpServer);
    }

    map<wstring, IJsonValue^>::const_iterator it = properties.find(TimeInfoNodeName L"." TimeZone);
    if (it != properties.end())
    {
        // ToDo: should we set wDayOfWeek to a non-zero value?

        TIME_ZONE_INFORMATION tzi = { 0 };

        JsonReader::TryFindNumber(properties, TimeInfoNodeName L"." TimeZone L"." TimeZoneBias, tzi.Bias);

        wstring standardName;
        if (JsonReader::TryFindString(properties, TimeInfoNodeName L"." TimeZone L"." TimeZoneStandardName, standardName))
        {
            wcsncpy_s(tzi.StandardName, standardName.c_str(), _TRUNCATE);
        }

        JsonReader::TryFindDateTime(properties, TimeInfoNodeName L"." TimeZone L"." TimeZoneStandardDate, tzi.StandardDate);

        JsonReader::TryFindNumber(properties, TimeInfoNodeName L"." TimeZone L"." TimeZoneStandardBias, tzi.StandardBias);

        wstring daylightName;
        if (JsonReader::TryFindString(properties, TimeInfoNodeName L"." TimeZone L"." TimeZoneDaylightName, daylightName))
        {
            wcsncpy_s(tzi.DaylightName, daylightName.c_str(), _TRUNCATE);
        }

        JsonReader::TryFindDateTime(properties, TimeInfoNodeName L"." TimeZone L"." TimeZoneDaylightDate, tzi.DaylightDate);

        JsonReader::TryFindNumber(properties, TimeInfoNodeName L"." TimeZone L"." TimeZoneDaylightBias, tzi.DaylightBias);

        if (!SetTimeZoneInformation(&tzi))
        {
            throw DMExceptionWithErrorCode("Error: failed to set time zone information. Error Code = ", GetLastError());
        }

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

std::wstring TimeCfg::GetTimeInfoJson()
{
    TRACE(__FUNCTION__);

    TimeInfo timeInfo;
    GetTimeInfo(timeInfo);

    JsonObject^ timeInfoJson = ref new JsonObject();

    timeInfoJson->Insert(ref new Platform::String(LocalTime),
                         JsonValue::CreateStringValue(ref new String(timeInfo.localTime.c_str())));

    timeInfoJson->Insert(ref new Platform::String(NtpServer),
                         JsonValue::CreateStringValue(ref new Platform::String(timeInfo.ntpServer.c_str())));

    JsonObject^ timeZoneJson = ref new JsonObject();

    timeZoneJson->Insert(ref new Platform::String(TimeZoneBias),
                        JsonValue::CreateNumberValue(static_cast<double>(timeInfo.timeZoneInformation.Bias)));

    timeZoneJson->Insert(ref new Platform::String(TimeZoneStandardName),
                        JsonValue::CreateStringValue(ref new Platform::String(timeInfo.timeZoneInformation.StandardName)));

    timeZoneJson->Insert(ref new Platform::String(TimeZoneStandardDate),
                        JsonValue::CreateStringValue(ref new Platform::String(Utils::ISO8601FromSystemTime(timeInfo.timeZoneInformation.StandardDate).c_str())));

    timeZoneJson->Insert(ref new Platform::String(TimeZoneStandardBias),
                        JsonValue::CreateNumberValue(static_cast<double>(timeInfo.timeZoneInformation.StandardBias)));

    timeZoneJson->Insert(ref new Platform::String(TimeZoneDaylightName),
                        JsonValue::CreateStringValue(ref new Platform::String(timeInfo.timeZoneInformation.DaylightName)));

    timeZoneJson->Insert(ref new Platform::String(TimeZoneDaylightDate),
                        JsonValue::CreateStringValue(ref new Platform::String(Utils::ISO8601FromSystemTime(timeInfo.timeZoneInformation.DaylightDate).c_str())));

    timeZoneJson->Insert(ref new Platform::String(TimeZoneDaylightBias),
                        JsonValue::CreateNumberValue(static_cast<double>(timeInfo.timeZoneInformation.DaylightBias)));

    timeInfoJson->Insert(ref new Platform::String(TimeZone), timeZoneJson);

    JsonObject^ root = ref new JsonObject();

    root->Insert(ref new String(TimeInfoNodeName), timeInfoJson);

    wstring json = root->Stringify()->Data();

    TRACEP(L" json = ", json.c_str());

    return json;
}
