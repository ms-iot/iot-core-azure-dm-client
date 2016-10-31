#pragma once
#include <string>

class TimeModel
{
public:
    static std::wstring NodeName();

    // Desired
    // ToDo: set time zone, etc

    // Reported
    Windows::Data::Json::JsonObject^ GetReportedProperties() const;
};