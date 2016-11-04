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
    void ExecWipe();

private:

    std::wstring _lastRemoteWipeCmdTime;    // Reported
};
