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

#include "stdafx.h"
#include <vector>
#include <map>
#include "TimeService.h"
#include "..\SharedUtilities\Utils.h"
#include "..\SharedUtilities\TimeHelpers.h"
#include "..\SharedUtilities\JsonHelpers.h"
#include "..\SharedUtilities\Logger.h"
#include "..\SharedUtilities\DMException.h"
#include "..\SharedUtilities\PolicyHelper.h"
#include "CSPs\MdmProvision.h"
#include "ServiceManager.h"
#include "..\DMShared\ErrorCodes.h"


#define TimeServiceName L"w32time"
#define JsonNo L"no"
#define JsonYes L"yes"
#define JsonNA L"n/a"
#define JsonAuto L"auto"
#define JsonManual L"manual"
#define JsonUnexpected L"unexpected"

#define MaxServiceStartWait 10

using namespace Windows::System;
using namespace Platform;
using namespace Windows::Data::Json;
using namespace Windows::System::Profile;
using namespace Windows::Foundation::Collections;

using namespace Microsoft::Devices::Management::Message;

using namespace std;
using namespace Utils;

TimeServiceData^ TimeService::GetState()
{
    TRACE(__FUNCTION__);

    TimeServiceData^ data = ref new TimeServiceData();

    if (ServiceManager::GetStartType(TimeServiceName) == SERVICE_DISABLED)
    {
        data->enabled = ref new String(JsonNo);
        data->startup = ref new String(JsonNA);
        data->started = ref new String(JsonNA);
    }
    else
    {
        data->enabled = ref new String(JsonYes);
        if (ServiceManager::GetStartType(TimeServiceName) == SERVICE_AUTO_START)
        {
            data->startup = ref new String(JsonAuto);
        }
        else if (ServiceManager::GetStartType(TimeServiceName) == SERVICE_DEMAND_START)
        {
            data->startup = ref new String(JsonManual);
        }
        else
        {
            data->startup = ref new String(JsonUnexpected);
        }

        DWORD status = ServiceManager::GetStatus(TimeServiceName);

        data->started = ref new String((status == SERVICE_RUNNING) ? JsonYes : JsonNo);
    }
    data->policy = PolicyHelper::LoadFromRegistry(RegTimeService);

    return data;
}

void TimeService::SaveState(TimeServiceData^ data)
{
    TRACE(__FUNCTION__);

    if (!data->policy)
    {
        throw DMExceptionWithErrorCode("Policy unspecified while storing Time Service settings", ERROR_DM_TIME_SERVICE_MISSING_POLICY);
    }

    // Save source priorities...
    PolicyHelper::SaveToRegistry(data->policy, RegTimeService);

    // Save remote/local properties...
    if (data->policy->source == PolicySource::Remote)
    {
        TRACE("Writing remote policy...");
        Utils::WriteRegistryValue(RegRemoteTimeService, RegTimeServiceEnabled, data->enabled->Data());
        Utils::WriteRegistryValue(RegRemoteTimeService, RegTimeServiceStartup, data->startup->Data());
        Utils::WriteRegistryValue(RegRemoteTimeService, RegTimeServiceStarted, data->started->Data());
    }
    else
    {
        TRACE("Writing local policy...");
        Utils::WriteRegistryValue(RegLocalTimeService, RegTimeServiceEnabled, data->enabled->Data());
        Utils::WriteRegistryValue(RegLocalTimeService, RegTimeServiceStartup, data->startup->Data());
        Utils::WriteRegistryValue(RegLocalTimeService, RegTimeServiceStarted, data->started->Data());
    }
}

TimeServiceData^ TimeService::GetActiveDesiredState()
{
    TRACE(__FUNCTION__);

    Policy^ policy = PolicyHelper::LoadFromRegistry(RegTimeService);
    if (!policy)
    {
        TRACE("Active desired state for Time Service is not set.");
        return nullptr;
    }

    for each(PolicySource p in policy->sourcePriorities)
    {
        wstring regSectionRoot = L"";
        switch (p)
        {
        case PolicySource::Local:
            TRACE("Reading local policy for Time Service.");
            regSectionRoot = RegLocalTimeService;
            break;
        case PolicySource::Remote:
            TRACE("Reading remote policy for Time Service.");
            regSectionRoot = RegRemoteTimeService;
            break;
        }

        bool success = true;

        wstring enabled;
        success &= ERROR_SUCCESS == Utils::TryReadRegistryValue(regSectionRoot.c_str(), RegTimeServiceEnabled, enabled);

        wstring startup;
        success &= ERROR_SUCCESS == Utils::TryReadRegistryValue(regSectionRoot.c_str(), RegTimeServiceStartup, startup);

        wstring started;
        success &= ERROR_SUCCESS == Utils::TryReadRegistryValue(regSectionRoot.c_str(), RegTimeServiceStarted, started);

        if (!success)
        {
            TRACE("Did not find one or more attributes in the registry.");

            // Try reading the next policy...
            continue;
        }

        TRACE("Found all Time Service attributes in the registry.");

        TimeServiceData^ data = ref new TimeServiceData();
        data->enabled = ref new String(enabled.c_str());
        data->startup = ref new String(startup.c_str());
        data->started = ref new String(started.c_str());
        data->policy = policy;
        return data;
    }

    TRACE("Could not find active desired state for Time Service.");
    return nullptr;
}

void TimeService::SetState(TimeServiceData^ data)
{
    TRACE(__FUNCTION__);

    SaveState(data);
    TimeServiceData^ activeDesiredState = GetActiveDesiredState();

    TRACE("Applying active desired state");
    if (activeDesiredState->enabled == JsonNo)
    {
        ServiceManager::Stop(TimeServiceName);
        ServiceManager::SetStartType(TimeServiceName, SERVICE_DISABLED);
    }
    else
    {
        if (activeDesiredState->startup == JsonAuto)
        {
            ServiceManager::SetStartType(TimeServiceName, SERVICE_AUTO_START);
        }
        else if (activeDesiredState->startup == JsonManual)
        {
            ServiceManager::SetStartType(TimeServiceName, SERVICE_DEMAND_START);
        }

        if (activeDesiredState->started == JsonYes)
        {
            ServiceManager::Start(TimeServiceName);
            ServiceManager::WaitStatus(TimeServiceName, SERVICE_RUNNING, MaxServiceStartWait);
        }
        else
        {
            ServiceManager::Stop(TimeServiceName);
            ServiceManager::WaitStatus(TimeServiceName, SERVICE_RUNNING, MaxServiceStartWait);
        }
    }
}
