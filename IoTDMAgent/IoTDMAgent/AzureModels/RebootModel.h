#pragma once
#include <string>
#include <mutex>

class RebootModel
{
public:
    RebootModel();
    static std::wstring NodeName();

    // Desired
    void SetDesiredProperties(Windows::Data::Json::IJsonValue^ rebootNode);

    // Reported
    Windows::Data::Json::JsonObject^ GetReportedProperties();

    // Operations
    void ExecRebootNow();

private:

    std::mutex _mutex;

    std::wstring _lastRebootCmdTime;    // Reported
    std::wstring _lastRestartTime;      // Reported
};