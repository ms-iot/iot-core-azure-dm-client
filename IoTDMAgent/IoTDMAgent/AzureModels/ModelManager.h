#pragma once

#include <string>
#include "RebootModel.h"
#include "RemoteWipeModel.h"
#include "TimeModel.h"
#include "SystemInfoModel.h"
#include "AzureUpdateManager.h"

class ModelManager
{
public:

    void ProcessDesiredProperties(bool completeSet, const std::string& allJson);

    std::string ExecRebootNow();
    std::string GetRebootPropertiesJson() const;

    std::string ExecWipeNow();
    std::string GetWipePropertiesJson() const;

    std::string GetAllPropertiesJson() const;

private:

    static Windows::Data::Json::IJsonValue^ GetDesiredPropertiesNode(bool completeSet, const std::string& allJson);
    void ProcessDesiredProperties(Windows::Data::Json::IJsonValue^ value);

    SystemInfoModel _systemInfoModel;
    TimeModel _timeModel;
    RebootModel _rebootModel;
    RemoteWipeModel _remoteWipeModel;
    AzureUpdateManager _azureUpdateManager;
};