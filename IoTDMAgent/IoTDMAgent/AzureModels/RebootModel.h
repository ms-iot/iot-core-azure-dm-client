#pragma once
#include <string>

class RebootModel
{
public:
    RebootModel();
    static std::wstring NodeName();

    // Desired
    void SetDesiredProperties(Windows::Data::Json::IJsonValue^ rebootNode);

    // Reported
    Windows::Data::Json::JsonObject^ GetReportedProperties() const;

    // Operations
    std::string ExecRebootNow();

private:

    std::wstring _lastRebootCmdTime;    // Reported
    std::wstring _lastRestartTime;      // Reported
};