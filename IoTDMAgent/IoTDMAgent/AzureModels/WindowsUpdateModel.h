#pragma once
#include <string>

class WindowsUpdateModel
{
public:
    static std::wstring NodeName();

    // Desired
    static void SetDesiredProperties(Windows::Data::Json::IJsonValue^ rebootNode);

    // Reported
    static Windows::Data::Json::JsonObject^ GetReportedProperties();
};