/*
Copyright 2017 Microsoft
Permission is hereby granted, free of charge, to any person obtaining a copy of this software 
and associated documentation files (the "Software"), to deal in the Software without restriction, 
including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, 
subject to the following conditions:

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT 
LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH 
THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#pragma once

#include <string>
#include "RebootModel.h"
#include "RemoteWipeModel.h"
#include "TimeModel.h"
#include "SystemInfoModel.h"
#include "AzureUpdateManager.h"
#include "WindowsUpdateModel.h"

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
    WindowsUpdateModel _windowsUpdateModel;
};
