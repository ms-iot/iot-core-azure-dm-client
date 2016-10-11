#include "stdafx.h"
#include <windows.h>
#include "TimeModel.h"

using namespace Windows::Data::Json;
using namespace Windows::Foundation::Collections;
using namespace Platform;
using namespace std;

#define TimeNode L"time"
#define CurrentTime L"current"
#define ZoneStandardName L"zoneStandardName"
#define ZoneDaylightName L"zoneDaylightName"

TimeModel::TimeModel()
{
    TRACE(L"TimeModel::TimeModel()");

    lock_guard<mutex> lock(_mutex);
}

wstring TimeModel::NodeName()
{
    return TimeNode;
}

Windows::Data::Json::JsonObject^ TimeModel::GetReportedProperties()
{
    TRACE(L"TimeModel::GetReportedProperties()");

    lock_guard<mutex> lock(_mutex);

    wstring zoneStandardName;
    wstring zoneDaylightName;
    TIME_ZONE_INFORMATION tzi;
    DWORD retCode = GetTimeZoneInformation(&tzi);
    if (TIME_ZONE_ID_INVALID != retCode)
    {
        zoneStandardName = tzi.StandardName;
        zoneDaylightName = tzi.DaylightName;
    }

    JsonObject^ timeProperties = ref new JsonObject();
    timeProperties->Insert(CurrentTime, JsonValue::CreateStringValue(ref new String(Utils::GetCurrentDateTimeString().c_str())));
    timeProperties->Insert(ZoneStandardName, JsonValue::CreateStringValue(ref new String(zoneStandardName.c_str())));
    timeProperties->Insert(ZoneDaylightName, JsonValue::CreateStringValue(ref new String(zoneDaylightName.c_str())));

    string jsonString = Utils::WideToMultibyte(timeProperties->Stringify()->Data());
    TRACEP("Time Model Json = ", jsonString.c_str());

    return timeProperties;
}
