#pragma once

#include <string>

class SystemInfoModel
{
public:
    SystemInfoModel();
    static std::wstring NodeName();

    // Reported
    Windows::Data::Json::JsonObject^ GetReportedProperties() const;
};