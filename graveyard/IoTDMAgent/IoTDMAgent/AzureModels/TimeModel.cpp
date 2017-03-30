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
#include <windows.h>
#include "TimeModel.h"

using namespace Windows::Data::Json;
using namespace Windows::Foundation::Collections;
using namespace Platform;
using namespace std;

#define TimeNode L"time"
#define CurrentTime L"current"
#define ZoneBias L"bias"
#define ZoneStandardName L"zoneStandardName"
#define ZoneStandardDate L"zoneStandardDate"
#define ZoneStandardBias L"zoneStandardBias"
#define ZoneDaylightName L"zoneDaylightName"
#define ZoneDaylightDate L"zoneDaylightDate"
#define ZoneDaylightBias L"zoneDaylightBias"

wstring TimeModel::NodeName()
{
    return TimeNode;
}

Windows::Data::Json::JsonObject^ TimeModel::GetReportedProperties() const
{
    TRACE(L"TimeModel::GetReportedProperties()");

    wstring zoneStandardName;
    wstring zoneDaylightName;
    TIME_ZONE_INFORMATION tzi;
    DWORD retCode = GetTimeZoneInformation(&tzi);
    if (TIME_ZONE_ID_INVALID != retCode)
    {
        zoneStandardName = tzi.StandardName;
        zoneDaylightName = tzi.DaylightName;
    }

    wstring zoneStandardDate = Utils::GetDateTimeString(tzi.StandardDate.wYear, tzi.StandardDate.wMonth, tzi.StandardDate.wDay, tzi.StandardDate.wHour, tzi.StandardDate.wMinute, tzi.StandardDate.wSecond);
    wstring zoneDaylightDate = Utils::GetDateTimeString(tzi.DaylightDate.wYear, tzi.DaylightDate.wMonth, tzi.DaylightDate.wDay, tzi.DaylightDate.wHour, tzi.DaylightDate.wMinute, tzi.DaylightDate.wSecond);

    JsonObject^ timeProperties = ref new JsonObject();
    timeProperties->Insert(CurrentTime, JsonValue::CreateStringValue(ref new String(Utils::GetCurrentDateTimeString().c_str())));
    timeProperties->Insert(ZoneBias, JsonValue::CreateNumberValue(tzi.Bias));
    timeProperties->Insert(ZoneStandardName, JsonValue::CreateStringValue(ref new String(zoneStandardName.c_str())));
    timeProperties->Insert(ZoneStandardDate, JsonValue::CreateStringValue(ref new String(zoneStandardDate.c_str())));
    timeProperties->Insert(ZoneStandardBias, JsonValue::CreateNumberValue(tzi.StandardBias));
    timeProperties->Insert(ZoneDaylightName, JsonValue::CreateStringValue(ref new String(zoneDaylightName.c_str())));
    timeProperties->Insert(ZoneDaylightDate, JsonValue::CreateStringValue(ref new String(zoneDaylightDate.c_str())));
    timeProperties->Insert(ZoneDaylightBias, JsonValue::CreateNumberValue(tzi.DaylightBias));

    string jsonString = Utils::WideToMultibyte(timeProperties->Stringify()->Data());
    TRACEP("Time Model Json = ", jsonString.c_str());

    return timeProperties;
}
