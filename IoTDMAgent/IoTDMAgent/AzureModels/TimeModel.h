#pragma once
#include <string>

class TimeModel
{
public:
    TimeModel();
    static std::wstring NodeName();

    // Desired
    // ToDo: set time zone, etc

    // Reported
    Windows::Data::Json::JsonObject^ GetReportedProperties() const;
};