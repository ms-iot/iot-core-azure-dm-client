#pragma once
#include <string>
#include <mutex>

class RemoteWipeModel
{
public:
    RemoteWipeModel();
    static std::wstring NodeName();

    // Reported
    Windows::Data::Json::JsonObject^ GetReportedProperties();

    // Operations
    std::string ExecWipe();

private:

    std::wstring _lastRemoteWipeCmdTime;    // Reported
};
