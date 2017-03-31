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
#include "WindowsUpdateModel.h"
#include "..\Utilities\Utils.h"
#include "..\LocalMachine\CSPs\PolicyCSP.h"
#include "..\LocalMachine\CSPs\UpdateCSP.h"

using namespace Windows::Data::Json;
using namespace Windows::Foundation::Collections;
using namespace Platform;
using namespace std;

#define WindowsUpdate L"windowsUpdate"
#define AllowAutoUpdate L"allowAutoUpdate"
#define ActiveHours L"activeHours"
#define AllowMUUpdates L"allowMUUpdates"
#define AllowNonMSsignedUpdates L"allowNonMSsignedUpdates"
#define AllowUpdateService L"allowUpdateService"
#define BranchReadinessLevel L"branchReadinessLevel"
#define DeferFeatureUpdates L"deferFeatureUpdates"
#define DeferQualityUpdates L"deferQualityUpdates"
#define ExcludeWUDrivers L"excludeWUDrivers"
#define PauseFeatureUpdates L"pauseFeatureUpdates"
#define PauseQualityUpdates L"pauseQualityUpdates"
#define RequireUpdateApproval L"requireUpdateApproval"
#define InstallDay L"installDay"
#define InstallHour L"installHour"
#define UpdateUrl L"updateUrl"
#define ApprovedUpdates L"approvedUpdates"
#define InstalledUpdates L"installedUpdates"
#define FailedUpdates L"failedUpdates"
#define InstallableUpdates L"installableUpdates"
#define PendingRebootUpdates L"pendingRebootUpdates"
#define LastSuccessfulScanTime L"lastSuccessfulScanTime"
#define DeferUpgrade L"deferUpgrade"

wstring WindowsUpdateModel::NodeName()
{
    return WindowsUpdate;
}

void WindowsUpdateModel::SetDesiredProperties(Windows::Data::Json::IJsonValue^ rebootNode)
{
    TRACE(__FUNCTION__);

    // Then, try to read them if the format is valid...
    JsonValueType type = rebootNode->ValueType;
    if (type == JsonValueType::Object)
    {
        JsonObject^ object = rebootNode->GetObject();
        if (object != nullptr)
        {
            for (IIterator<IKeyValuePair<String^, IJsonValue^>^>^ iter = object->First();
                iter->HasCurrent;
                iter->MoveNext())
            {
                IKeyValuePair<String^, IJsonValue^>^ pair = iter->Current;
                String^ childKey = pair->Key;

                if (pair->Value->ValueType == JsonValueType::Number)
                {
                    unsigned int intValue = static_cast<unsigned int>(pair->Value->GetNumber());

                    if (childKey == AllowAutoUpdate)
                    {
                        PolicyCSP::SetAllowAutoUpdate(intValue);
                    }
                    else if (childKey == BranchReadinessLevel)
                    {
                        PolicyCSP::SetBranchReadinessLevel(intValue);
                    }
                    else if (childKey == DeferFeatureUpdates)
                    {
                        PolicyCSP::SetDeferFeatureUpdatesPeriodInDays(intValue);
                    }
                    else if (childKey == DeferQualityUpdates)
                    {
                        PolicyCSP::SetDeferQualityUpdatesPeriodInDays(intValue);
                    }
                    else if (childKey == InstallDay)
                    {
                        PolicyCSP::SetScheduledInstallDay(intValue);
                    }
                    else if (childKey == InstallHour)
                    {
                        PolicyCSP::SetScheduledInstallTime(intValue);
                    }
                    else if (childKey == AllowMUUpdates)
                    {
                        PolicyCSP::SetAllowMUUpdateService(intValue);
                    }
                    else if (childKey == AllowNonMSsignedUpdates)
                    {
                        PolicyCSP::SetAllowNonMicrosoftSignedUpdate(intValue);
                    }
                    else if (childKey == AllowUpdateService)
                    {
                        PolicyCSP::SetAllowUpdateService(intValue);
                    }
                    else if (childKey == ExcludeWUDrivers)
                    {
#if 0
                        // ToDo: Need to investigate why this is failing (if we need it).
                        // (Recipient Exception) Command failed. The recipient encountered an unexpected condition which prevented it from fulfilling the request
                        PolicyCSP::SetExcludeWUDrivers(intValue);
#endif
                    }
                    else if (childKey == PauseFeatureUpdates)
                    {
                        PolicyCSP::SetPauseFeatureUpdates(intValue);
                    }
                    else if (childKey == PauseQualityUpdates)
                    {
                        PolicyCSP::SetPauseQualityUpdates(intValue);
                    }
                    else if (childKey == RequireUpdateApproval)
                    {
                        PolicyCSP::SetRequireUpdateApproval(intValue);
                    }
                }
                else if (pair->Value->ValueType == JsonValueType::String)
                {
                    wstring stringValue = pair->Value->GetString()->Data();
                    if (childKey == ActiveHours)
                    {
                        vector<wstring> tokens;
                        Utils::SplitString(stringValue, L'-', tokens);
                        if (tokens.size() == 2)
                        {
                            PolicyCSP::SetActiveHoursStart(stoi(tokens[0]));
                            PolicyCSP::SetActiveHoursEnd(stoi(tokens[1]));
                        }
                        else
                        {
                            throw DMException("Incorrect active hours format.");
                        }
                    }
                    else if (childKey == UpdateUrl)
                    {
                        PolicyCSP::SetUpdateServiceUrl(stringValue);
                    }
                    else if (childKey == ApprovedUpdates)
                    {
                        vector<wstring> tokens;
                        Utils::SplitString(stringValue, L'\\', tokens);
                        for (const wstring& s : tokens)
                        {
                            UpdateCSP::AddApprovedUpdates(s);
                        }
                    }
                }
            }
        }
        else
        {
            TRACE("Warning: Unexpected reboot properties contents. Skipping.");
            return;
        }
    }
    else
    {
        TRACE("Warning: Unexpected reboot properties contents. Skipping.");
    }
}

JsonObject^ WindowsUpdateModel::GetReportedProperties()
{
    TRACE(__FUNCTION__);

    JsonObject^ windowsUpdateProperties = ref new JsonObject();
    windowsUpdateProperties->Insert(AllowAutoUpdate, JsonValue::CreateNumberValue(static_cast<double>(PolicyCSP::GetAllowAutoUpdate())));

    wstring activeHoursStart = to_wstring(PolicyCSP::GetActiveHoursStart());
    wstring activeHoursEnd = to_wstring(PolicyCSP::GetActiveHoursEnd());
    wstring activeHours = activeHoursStart + L"-" + activeHoursEnd;

    windowsUpdateProperties->Insert(ActiveHours, JsonValue::CreateStringValue(ref new String(activeHours.c_str())));
    windowsUpdateProperties->Insert(AllowMUUpdates, JsonValue::CreateNumberValue(static_cast<double>(PolicyCSP::GetAllowMUUpdateService())));
    windowsUpdateProperties->Insert(AllowNonMSsignedUpdates, JsonValue::CreateNumberValue(static_cast<double>(PolicyCSP::GetAllowNonMicrosoftSignedUpdate())));
    windowsUpdateProperties->Insert(AllowUpdateService, JsonValue::CreateNumberValue(static_cast<double>(PolicyCSP::GetAllowUpdateService())));
    windowsUpdateProperties->Insert(BranchReadinessLevel, JsonValue::CreateNumberValue(static_cast<double>(PolicyCSP::GetBranchReadinessLevel())));
    windowsUpdateProperties->Insert(DeferFeatureUpdates, JsonValue::CreateNumberValue(static_cast<double>(PolicyCSP::GetDeferFeatureUpdatesPeriodInDays())));
    windowsUpdateProperties->Insert(DeferQualityUpdates, JsonValue::CreateNumberValue(static_cast<double>(PolicyCSP::GetDeferQualityUpdatesPeriodInDays())));
#if 0
    // ToDo: Need to investigate why this is failing (if we need it).
    // (Originator Exception) Not found.The requested target was not found.No indication is given as to whether this is a temporary or permanent condition.The response code 410 SHOULD be
    // used when the condition is permanent and the recipient wishes to make this fact public.This response code is also used when the recipient does not want to make public the reason fo
    // r why a requested command is not allowed or when no other response code is appropriate.
    windowsUpdateProperties->Insert(ExcludeWUDrivers, JsonValue::CreateNumberValue(static_cast<double>(PolicyCSP::GetExcludeWUDrivers())));
#endif
    windowsUpdateProperties->Insert(PauseFeatureUpdates, JsonValue::CreateNumberValue(static_cast<double>(PolicyCSP::GetPauseFeatureUpdates())));
    windowsUpdateProperties->Insert(PauseQualityUpdates, JsonValue::CreateNumberValue(static_cast<double>(PolicyCSP::GetPauseQualityUpdates())));
    windowsUpdateProperties->Insert(RequireUpdateApproval, JsonValue::CreateNumberValue(static_cast<double>(PolicyCSP::GetRequireUpdateApproval())));
    windowsUpdateProperties->Insert(InstallDay, JsonValue::CreateNumberValue(static_cast<double>(PolicyCSP::GetScheduledInstallDay())));
    windowsUpdateProperties->Insert(InstallHour, JsonValue::CreateNumberValue(static_cast<double>(PolicyCSP::GetScheduledInstallTime())));
    windowsUpdateProperties->Insert(UpdateUrl, JsonValue::CreateStringValue(ref new String(PolicyCSP::GetUpdateServiceUrl().c_str())));

    windowsUpdateProperties->Insert(ApprovedUpdates, JsonValue::CreateStringValue(ref new String(UpdateCSP::GetApprovedUpdates().c_str())));
    windowsUpdateProperties->Insert(InstalledUpdates, JsonValue::CreateStringValue(ref new String(UpdateCSP::GetInstalledUpdates().c_str())));
    windowsUpdateProperties->Insert(FailedUpdates, JsonValue::CreateStringValue(ref new String(UpdateCSP::GetFailedUpdates().c_str())));
    windowsUpdateProperties->Insert(InstallableUpdates, JsonValue::CreateStringValue(ref new String(UpdateCSP::GetInstallableUpdates().c_str())));
    windowsUpdateProperties->Insert(PendingRebootUpdates, JsonValue::CreateStringValue(ref new String(UpdateCSP::GetPendingRebootUpdates().c_str())));
    windowsUpdateProperties->Insert(LastSuccessfulScanTime, JsonValue::CreateStringValue(ref new String(UpdateCSP::GetLastSuccessfulScanTime().c_str())));
    windowsUpdateProperties->Insert(DeferUpgrade, JsonValue::CreateBooleanValue(UpdateCSP::GetDeferUpgrade()));

    string jsonString = Utils::WideToMultibyte(windowsUpdateProperties->Stringify()->Data());
    TRACEP("Windows Update Model Json = ", jsonString.c_str());

    return windowsUpdateProperties;
}
