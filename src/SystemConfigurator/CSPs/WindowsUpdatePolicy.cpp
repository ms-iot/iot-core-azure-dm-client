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

#include <stdafx.h>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include "..\SharedUtilities\Logger.h"
#include "..\SharedUtilities\Utils.h"
#include "..\SharedUtilities\PolicyHelper.h"
#include "..\DMShared\ErrorCodes.h"
#include "MdmProvision.h"
#include "WindowsUpdatePolicyCSP.h"
#include "Permissions\PermissionsManager.h"

using namespace std;
using namespace Microsoft::Devices::Management::Message;
using namespace Utils;

const wchar_t* WURingRegistrySubKey = L"SYSTEM\\Platform\\DeviceTargetingInfo";
const wchar_t* WURingPropertyName = L"TargetRing";

bool WindowsUpdatePolicyCSP::IsRingSupported()
{
    static bool checked = false;
    static bool supported = false;

    if (!checked)
    {
        supported = RegistryKeyExists(WURingRegistrySubKey);
        checked = true;
    }
    return supported;
}

IResponse^ WindowsUpdatePolicyCSP::Get(IRequest^ request)
{
    TRACE(__FUNCTION__);

    // Set default values...
    wstring reportToDeviceTwin = JsonNo->Data();

    unsigned int activeHoursStart = static_cast<unsigned int>(-1);
    unsigned int activeHoursEnd = static_cast<unsigned int>(-1);
    unsigned int allowAutoUpdate = static_cast<unsigned int>(-1);


    unsigned int allowUpdateService = static_cast<unsigned int>(-1);
    unsigned int branchReadinessLevel = static_cast<unsigned int>(-1);
    unsigned int deferFeatureUpdatesPeriod = static_cast<unsigned int>(-1);    // in days
    unsigned int deferQualityUpdatesPeriod = static_cast<unsigned int>(-1);    // in days

    unsigned int pauseFeatureUpdates = static_cast<unsigned int>(-1);
    unsigned int pauseQualityUpdates = static_cast<unsigned int>(-1);
    unsigned int scheduledInstallDay = static_cast<unsigned int>(-1);
    unsigned int scheduledInstallTime = static_cast<unsigned int>(-1);

    wstring ring = L"<error reading ring>";

    // Read the values...
    MdmProvision::TryGetNumber<unsigned int>(L"./Device/Vendor/MSFT/Policy/Result/Update/ActiveHoursStart", activeHoursStart);
    MdmProvision::TryGetNumber<unsigned int>(L"./Device/Vendor/MSFT/Policy/Result/Update/ActiveHoursEnd", activeHoursEnd);
    MdmProvision::TryGetNumber<unsigned int>(L"./Device/Vendor/MSFT/Policy/Result/Update/AllowAutoUpdate", allowAutoUpdate);

    MdmProvision::TryGetNumber<unsigned int>(L"./Device/Vendor/MSFT/Policy/Result/Update/AllowUpdateService", allowUpdateService);
    MdmProvision::TryGetNumber<unsigned int>(L"./Device/Vendor/MSFT/Policy/Result/Update/BranchReadinessLevel", branchReadinessLevel);
    MdmProvision::TryGetNumber<unsigned int>(L"./Device/Vendor/MSFT/Policy/Result/Update/DeferFeatureUpdatesPeriodInDays", deferFeatureUpdatesPeriod);
    MdmProvision::TryGetNumber<unsigned int>(L"./Device/Vendor/MSFT/Policy/Result/Update/DeferQualityUpdatesPeriodInDays", deferQualityUpdatesPeriod);

    MdmProvision::TryGetNumber<unsigned int>(L"./Device/Vendor/MSFT/Policy/Result/Update/PauseFeatureUpdates", pauseFeatureUpdates);
    MdmProvision::TryGetNumber<unsigned int>(L"./Device/Vendor/MSFT/Policy/Result/Update/PauseQualityUpdates", pauseQualityUpdates);
    MdmProvision::TryGetNumber<unsigned int>(L"./Device/Vendor/MSFT/Policy/Result/Update/ScheduledInstallDay", scheduledInstallDay);
    MdmProvision::TryGetNumber<unsigned int>(L"./Device/Vendor/MSFT/Policy/Result/Update/ScheduledInstallTime", scheduledInstallTime);

    if (IsRingSupported())
    {
        Utils::TryReadRegistryValue(WURingRegistrySubKey, WURingPropertyName, ring);
    }
    else
    {
        ring = L"<ring setting is not supported>";
    }

    Utils::TryReadRegistryValue(IoTDMRegistryRoot, RegWindowsUpdatePolicySectionReporting, reportToDeviceTwin);

    // Populate the response...
    auto data = ref new WindowsUpdatePolicyConfiguration();

    data->activeFields = 0xFFFFFFFF;   // We report all fields.

    data->activeHoursStart = activeHoursStart;
    data->activeHoursEnd = activeHoursEnd;
    data->allowAutoUpdate = allowAutoUpdate;

    data->allowUpdateService = allowUpdateService;
    data->branchReadinessLevel = branchReadinessLevel;
    data->deferFeatureUpdatesPeriod = deferFeatureUpdatesPeriod;
    data->deferQualityUpdatesPeriod = deferQualityUpdatesPeriod;

    data->pauseFeatureUpdates = pauseFeatureUpdates;
    data->pauseQualityUpdates = pauseQualityUpdates;
    data->scheduledInstallDay = scheduledInstallDay;
    data->scheduledInstallTime = scheduledInstallTime;

    data->ring = ref new String(ring.c_str());
    data->policy = PolicyHelper::LoadFromRegistry(RegWindowsUpdatePolicy);

    auto configuration = ref new GetWindowsUpdatePolicyResponse(ResponseStatus::Success, data);
    configuration->ReportToDeviceTwin = ref new String(reportToDeviceTwin.c_str());

    return configuration;
}

void WindowsUpdatePolicyCSP::SaveState(WindowsUpdatePolicyConfiguration^ data)
{
    TRACE(__FUNCTION__);

    if (!data->policy)
    {
        throw DMExceptionWithErrorCode("Policy unspecified while storing Windows Update policy settings", ERROR_DM_WINDOWS_UPDATE_POLICY_MISSING_POLICY);
    }

    // Save source priorities...
    PolicyHelper::SaveToRegistry(data->policy, RegWindowsUpdatePolicy);

    const wchar_t* registryRoot = nullptr;
    if (data->policy->source == PolicySource::Remote)
    {
        TRACE("Writing remote policy...");
        registryRoot = RegRemoteWindowsUpdatePolicy;
    }
    else
    {
        TRACE("Writing local policy...");
        registryRoot = RegLocalWindowsUpdatePolicy;
    }

    // Save remote/local properties...
    if (data->activeFields & (unsigned int)ActiveFields::ActiveHoursStart)
        Utils::WriteRegistryValue(registryRoot, RegWindowsUpdateActiveHoursStart, data->activeHoursStart);

    if (data->activeFields & (unsigned int)ActiveFields::ActiveHoursEnd)
        Utils::WriteRegistryValue(registryRoot, RegWindowsUpdateActiveHoursEnd, data->activeHoursEnd);

    if (data->activeFields & (unsigned int)ActiveFields::AllowAutoUpdate)
        Utils::WriteRegistryValue(registryRoot, RegWindowsUpdateAllowAutoUpdate, data->allowAutoUpdate);

    if (data->activeFields & (unsigned int)ActiveFields::AllowUpdateService)
        Utils::WriteRegistryValue(registryRoot, RegWindowsUpdateAllowUpdateService, data->allowUpdateService);

    if (data->activeFields & (unsigned int)ActiveFields::BranchReadinessLevel)
        Utils::WriteRegistryValue(registryRoot, RegWindowsUpdateBranchReadinessLevel, data->branchReadinessLevel);

    if (data->activeFields & (unsigned int)ActiveFields::DeferFeatureUpdatesPeriod)
        Utils::WriteRegistryValue(registryRoot, RegWindowsUpdateDeferFeatureUpdatesPeriod, data->deferFeatureUpdatesPeriod);

    if (data->activeFields & (unsigned int)ActiveFields::DeferQualityUpdatesPeriod)
        Utils::WriteRegistryValue(registryRoot, RegWindowsUpdateDeferQualityUpdatesPeriod, data->deferQualityUpdatesPeriod);

    if (data->activeFields & (unsigned int)ActiveFields::PauseFeatureUpdates)
        Utils::WriteRegistryValue(registryRoot, RegWindowsUpdatePauseFeatureUpdates, data->pauseFeatureUpdates);

    if (data->activeFields & (unsigned int)ActiveFields::PauseQualityUpdates)
        Utils::WriteRegistryValue(registryRoot, RegWindowsUpdatePauseQualityUpdates, data->pauseQualityUpdates);

    if (data->activeFields & (unsigned int)ActiveFields::ScheduledInstallDay)
        Utils::WriteRegistryValue(registryRoot, RegWindowsUpdateScheduledInstallDay, data->scheduledInstallDay);

    if (data->activeFields & (unsigned int)ActiveFields::ScheduledInstallTime)
        Utils::WriteRegistryValue(registryRoot, RegWindowsUpdateScheduledInstallTime, data->scheduledInstallTime);

    if (data->activeFields & (unsigned int)ActiveFields::Ring)
    {
        if (IsRingSupported())
        {
            Utils::WriteRegistryValue(registryRoot, RegWindowsUpdatePolicyRing, data->ring->Data());
        }
        else
        {
            // Ring is not supported on IoT Enterprise.
            throw DMExceptionWithErrorCode("Ring is not supported on this platform.", ERROR_DM_WINDOWS_UPDATE_POLICY_RING_NOT_SUPPORTED);
        }
    }
}

WindowsUpdatePolicyConfiguration^ WindowsUpdatePolicyCSP::GetActiveDesiredState()
{
    TRACE(__FUNCTION__);

    Policy^ policy = PolicyHelper::LoadFromRegistry(RegWindowsUpdatePolicy);
    if (!policy)
    {
        TRACE("Active desired state for Windows Update Policy is not set.");
        return nullptr;
    }

    for each(PolicySource p in policy->sourcePriorities)
    {
        wstring regSectionRoot = L"";
        switch (p)
        {
        case PolicySource::Local:
            TRACE("Reading local policy for Windows Update Policy.");
            regSectionRoot = RegLocalWindowsUpdatePolicy;
            break;
        case PolicySource::Remote:
            TRACE("Reading remote policy for Windows Update Policy.");
            regSectionRoot = RegRemoteWindowsUpdatePolicy;
            break;
        }

        unsigned int activeFields = 0;

        wstring ring;
        if (ERROR_SUCCESS == Utils::TryReadRegistryValue(regSectionRoot.c_str(), RegWindowsUpdatePolicyRing, ring))
        {
            activeFields |= (unsigned int)ActiveFields::Ring;
        }

        unsigned long activeHoursEnd;
        if (ERROR_SUCCESS == Utils::TryReadRegistryValue(regSectionRoot.c_str(), RegWindowsUpdateActiveHoursEnd, activeHoursEnd))
        {
            activeFields |= (unsigned int)ActiveFields::ActiveHoursEnd;
        }

        unsigned long activeHoursStart;
        if (ERROR_SUCCESS == Utils::TryReadRegistryValue(regSectionRoot.c_str(), RegWindowsUpdateActiveHoursStart, activeHoursStart))
        {
            activeFields |= (unsigned int)ActiveFields::ActiveHoursStart;
        }

        unsigned long allowAutoUpdate;
        if (ERROR_SUCCESS == Utils::TryReadRegistryValue(regSectionRoot.c_str(), RegWindowsUpdateAllowAutoUpdate, allowAutoUpdate))
        {
            activeFields |= (unsigned int)ActiveFields::AllowAutoUpdate;
        }

        unsigned long allowUpdateService;
        if (ERROR_SUCCESS == Utils::TryReadRegistryValue(regSectionRoot.c_str(), RegWindowsUpdateAllowUpdateService, allowUpdateService))
        {
            activeFields |= (unsigned int)ActiveFields::AllowUpdateService;
        }

        unsigned long branchReadinessLevel;
        if (ERROR_SUCCESS == Utils::TryReadRegistryValue(regSectionRoot.c_str(), RegWindowsUpdateBranchReadinessLevel, branchReadinessLevel))
        {
            activeFields |= (unsigned int)ActiveFields::BranchReadinessLevel;
        }

        unsigned long deferFeatureUpdatesPeriod;
        if (ERROR_SUCCESS == Utils::TryReadRegistryValue(regSectionRoot.c_str(), RegWindowsUpdateDeferFeatureUpdatesPeriod, deferFeatureUpdatesPeriod))
        {
            activeFields |= (unsigned int)ActiveFields::DeferFeatureUpdatesPeriod;
        }

        unsigned long deferQualityUpdatesPeriod;
        if (ERROR_SUCCESS == Utils::TryReadRegistryValue(regSectionRoot.c_str(), RegWindowsUpdateDeferQualityUpdatesPeriod, deferQualityUpdatesPeriod))
        {
            activeFields |= (unsigned int)ActiveFields::DeferQualityUpdatesPeriod;
        }

        unsigned long pauseFeatureUpdates;
        if (ERROR_SUCCESS == Utils::TryReadRegistryValue(regSectionRoot.c_str(), RegWindowsUpdatePauseFeatureUpdates, pauseFeatureUpdates))
        {
            activeFields |= (unsigned int)ActiveFields::PauseFeatureUpdates;
        }

        unsigned long pauseQualityUpdates;
        if (ERROR_SUCCESS == Utils::TryReadRegistryValue(regSectionRoot.c_str(), RegWindowsUpdatePauseQualityUpdates, pauseQualityUpdates))
        {
            activeFields |= (unsigned int)ActiveFields::PauseQualityUpdates;
        }

        unsigned long scheduledInstallDay;
        if (ERROR_SUCCESS == Utils::TryReadRegistryValue(regSectionRoot.c_str(), RegWindowsUpdateScheduledInstallDay, scheduledInstallDay))
        {
            activeFields |= (unsigned int)ActiveFields::ScheduledInstallDay;
        }

        unsigned long scheduledInstallTime;
        if (ERROR_SUCCESS == Utils::TryReadRegistryValue(regSectionRoot.c_str(), RegWindowsUpdateScheduledInstallTime, scheduledInstallTime))
        {
            activeFields |= (unsigned int)ActiveFields::ScheduledInstallTime;
        }

        if (activeFields == 0)
        {
            TRACE("Did not find any attributes in the registry.");

            // Try reading the next policy...
            continue;
        }

        TRACE("Found all Windows Update attributes in the registry.");

        WindowsUpdatePolicyConfiguration^ activeData = ref new WindowsUpdatePolicyConfiguration();

        activeData->activeFields = activeFields;

        activeData->ring = ref new String(ring.c_str());
        activeData->activeHoursEnd = activeHoursEnd;
        activeData->activeHoursStart = activeHoursStart;
        activeData->allowAutoUpdate = allowAutoUpdate;
        activeData->allowUpdateService = allowUpdateService;
        activeData->branchReadinessLevel = branchReadinessLevel;
        activeData->deferFeatureUpdatesPeriod = deferFeatureUpdatesPeriod;
        activeData->deferQualityUpdatesPeriod = deferQualityUpdatesPeriod;
        activeData->pauseFeatureUpdates = pauseFeatureUpdates;
        activeData->pauseQualityUpdates = pauseQualityUpdates;
        activeData->scheduledInstallDay = scheduledInstallDay;
        activeData->scheduledInstallTime = scheduledInstallTime;

        activeData->policy = policy;
        return activeData;
    }

    TRACE("Could not find active desired state for Windows Update Policy.");
    return nullptr;
}

void WindowsUpdatePolicyCSP::HandleSetDesiredProperties(WindowsUpdatePolicyConfiguration^ data)
{
    // This will save active fields in the registry hive specified by the policy.
    SaveState(data);

    // This will read the settings from the active hive, and set activeFields to those which were found...
    // This set returned is the union o f the active fields in this call and previous set calls.
    // We will used the returned active fields (the union) because this call can also be used to switch
    // the priority from local to remote (or vice versa). In such a case, we want to apply everything that
    // has already been defined in that set.
    //
    WindowsUpdatePolicyConfiguration^ activeDesiredState = GetActiveDesiredState();

    if (activeDesiredState == nullptr || activeDesiredState->policy == nullptr)
    {
        // This would take place if 'data' does not have a policy, or it has no active fields.
        // The trace should have information on what toke place.
        return;
    }

    if (activeDesiredState->policy->source != data->policy->source)
    {
        // No need to apply this set...
        return;
    }

    unsigned int activeFields = activeDesiredState->activeFields;

    if (activeFields & (unsigned int)ActiveFields::ActiveHoursStart)
        MdmProvision::RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/ActiveHoursStart", static_cast<int>(activeDesiredState->activeHoursStart));

    if (activeFields & (unsigned int)ActiveFields::ActiveHoursEnd)
        MdmProvision::RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/ActiveHoursEnd", static_cast<int>(activeDesiredState->activeHoursEnd));

    if (activeFields & (unsigned int)ActiveFields::AllowAutoUpdate)
        MdmProvision::RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/AllowAutoUpdate", static_cast<int>(activeDesiredState->allowAutoUpdate));

    if (activeFields & (unsigned int)ActiveFields::AllowUpdateService)
        MdmProvision::RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/AllowUpdateService", static_cast<int>(activeDesiredState->allowUpdateService));

    if (activeFields & (unsigned int)ActiveFields::BranchReadinessLevel)
        MdmProvision::RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/BranchReadinessLevel", static_cast<int>(activeDesiredState->branchReadinessLevel));

    if (activeFields & (unsigned int)ActiveFields::DeferFeatureUpdatesPeriod)
        MdmProvision::RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/DeferFeatureUpdatesPeriodInDays", static_cast<int>(activeDesiredState->deferFeatureUpdatesPeriod));

    if (activeFields & (unsigned int)ActiveFields::DeferQualityUpdatesPeriod)
        MdmProvision::RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/DeferQualityUpdatesPeriodInDays", static_cast<int>(activeDesiredState->deferQualityUpdatesPeriod));

    if (activeFields & (unsigned int)ActiveFields::PauseFeatureUpdates)
        MdmProvision::RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/PauseFeatureUpdates", static_cast<int>(activeDesiredState->pauseFeatureUpdates));

    if (activeFields & (unsigned int)ActiveFields::PauseQualityUpdates)
        MdmProvision::RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/PauseQualityUpdates", static_cast<int>(activeDesiredState->pauseQualityUpdates));

    if (activeFields & (unsigned int)ActiveFields::ScheduledInstallDay)
        MdmProvision::RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/ScheduledInstallDay", static_cast<int>(activeDesiredState->scheduledInstallDay));

    if (activeFields & (unsigned int)ActiveFields::ScheduledInstallTime)
        MdmProvision::RunSet(L"./Device/Vendor/MSFT/Policy/Config/Update/ScheduledInstallTime", static_cast<int>(activeDesiredState->scheduledInstallTime));

    if (activeFields & (unsigned int)ActiveFields::Ring)
    {
        if (IsRingSupported())
        {
            wstring registryRoot = L"MACHINE";
            wstring registryKey = registryRoot + L"\\" + WURingRegistrySubKey;
            wstring propertyValue = activeDesiredState->ring->Data();

            PermissionsManager::ModifyProtected(registryKey, SE_REGISTRY_KEY, [propertyValue]()
            {
                TRACEP(L"........Writing registry: key name: ", WURingRegistrySubKey);
                TRACEP(L"........Writing registry: key value: ", propertyValue.c_str());
                Utils::WriteRegistryValue(WURingRegistrySubKey, WURingPropertyName, propertyValue);
            });
        }
        else
        {
            // Ring is not supported on IoT Enterprise.
            throw DMExceptionWithErrorCode("Ring is not supported on this platform.", ERROR_DM_WINDOWS_UPDATE_POLICY_RING_NOT_SUPPORTED);
        }
    }
}

IResponse^ WindowsUpdatePolicyCSP::Set(IRequest^ request)
{
    // ToDo: We need to have a consistent policy on whether we:
    // - apply all or nothing.
    // - apply as much as we can and report an error.

    auto updatePolicyRequest = dynamic_cast<SetWindowsUpdatePolicyRequest^>(request);
    WindowsUpdatePolicyConfiguration^ data = updatePolicyRequest->data;

    if (data != nullptr && updatePolicyRequest->ApplyFromDeviceTwin == JsonYes)
    {
        HandleSetDesiredProperties(data);
    }
    if (updatePolicyRequest->ReportToDeviceTwin != ValueUnspecified)
    {
        Utils::WriteRegistryValue(IoTDMRegistryRoot, RegWindowsUpdatePolicySectionReporting, updatePolicyRequest->ReportToDeviceTwin->Data());
    }

    return ref new StatusCodeResponse(ResponseStatus::Success, request->Tag);
}
