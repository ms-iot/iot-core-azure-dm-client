#pragma once

#include <string>
#include <windows.h>

// ToDo: Need to rethink these helpers. Ideally, replace with an existing library.

namespace Utils
{
    struct ISO8601DateTime
    {
        unsigned short year;
        unsigned short month;
        unsigned short day;
        unsigned short hour;
        unsigned short minute;
        unsigned short second;
        unsigned short milliseconds;
        short zoneHour;
        unsigned short zoneMinute;
    };

    bool SystemTimeFromISO8601(const std::wstring& dateTimeString, SYSTEMTIME& dateTime);
    std::wstring ISO8601FromSystemTime(const SYSTEMTIME& dateTime);

    bool ISO8601DateTimeFromString(const std::wstring& dateTimeString, ISO8601DateTime& dateTime);
    std::wstring StringFromISO8601DateTime(const ISO8601DateTime& dateTime);

    std::wstring CanonicalizeDateTime(const std::wstring& dateTimeString);
}