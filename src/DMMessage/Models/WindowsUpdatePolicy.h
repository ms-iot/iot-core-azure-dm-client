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
#include "IRequestIResponse.h"
#include "SerializationHelper.h"
#include "DMMessageKind.h"
#include "StatusCodeResponse.h"
#include "Blob.h"
#include "ModelHelper.h"

using namespace Platform;
using namespace Platform::Metadata;
using namespace Windows::Data::Json;

namespace Microsoft { namespace Devices { namespace Management { namespace Message
{
    public enum class ActiveFields
    {
        ActiveHoursStart = 0x0001,
        ActiveHoursEnd = 0x0002,
        AllowAutoUpdate = 0x0004,
        AllowUpdateService = 0x0008,
        BranchReadinessLevel = 0x0010,
        DeferFeatureUpdatesPeriod = 0x0020,
        DeferQualityUpdatesPeriod = 0x0040,
        PauseFeatureUpdates = 0x0080,
        PauseQualityUpdates = 0x0100,
        ScheduledInstallDay = 0x0200,
        ScheduledInstallTime = 0x0400,
        Ring = 0x0800,
    };

    [Windows::Foundation::Metadata::WebHostHidden]
    public ref class WindowsUpdatePolicyConfiguration sealed
    {
    public:
        property unsigned int activeFields;

        // Optional fields.
        property unsigned int activeHoursStart;
        property unsigned int activeHoursEnd;
        property unsigned int allowAutoUpdate;
        property unsigned int allowUpdateService;
        property unsigned int branchReadinessLevel;

        property unsigned int deferFeatureUpdatesPeriod;    // in days
        property unsigned int deferQualityUpdatesPeriod;    // in days
        property unsigned int pauseFeatureUpdates;
        property unsigned int pauseQualityUpdates;
        property unsigned int scheduledInstallDay;

        property unsigned int scheduledInstallTime;

        property String^ ring;

        WindowsUpdatePolicyConfiguration()
        {
            activeFields = 0;

            activeHoursStart = 0;
            activeHoursEnd = 0;
            allowAutoUpdate = 0;
            allowUpdateService = 0;
            branchReadinessLevel = 0;

            deferFeatureUpdatesPeriod = 0;    // in days
            deferQualityUpdatesPeriod = 0;    // in days
            pauseFeatureUpdates = 0;
            pauseQualityUpdates = 0;
            scheduledInstallDay = 0;

            scheduledInstallTime = 0;

            ring = ref new String();
        }

        JsonObject^ ToJsonObject(JsonObject^ jsonObject)
        {
            if (activeFields & (unsigned int)ActiveFields::ActiveHoursStart)
                jsonObject->Insert("activeHoursStart", JsonValue::CreateNumberValue(activeHoursStart));

            if (activeFields & (unsigned int)ActiveFields::ActiveHoursEnd)
                jsonObject->Insert("activeHoursEnd", JsonValue::CreateNumberValue(activeHoursEnd));

            if (activeFields & (unsigned int)ActiveFields::AllowAutoUpdate)
                jsonObject->Insert("allowAutoUpdate", JsonValue::CreateNumberValue(allowAutoUpdate));

            if (activeFields & (unsigned int)ActiveFields::AllowUpdateService)
                jsonObject->Insert("allowUpdateService", JsonValue::CreateNumberValue(allowUpdateService));

            if (activeFields & (unsigned int)ActiveFields::BranchReadinessLevel)
                jsonObject->Insert("branchReadinessLevel", JsonValue::CreateNumberValue(branchReadinessLevel));

            if (activeFields & (unsigned int)ActiveFields::DeferFeatureUpdatesPeriod)
                jsonObject->Insert("deferFeatureUpdatesPeriod", JsonValue::CreateNumberValue(deferFeatureUpdatesPeriod));

            if (activeFields & (unsigned int)ActiveFields::DeferQualityUpdatesPeriod)
                jsonObject->Insert("deferQualityUpdatesPeriod", JsonValue::CreateNumberValue(deferQualityUpdatesPeriod));

            if (activeFields & (unsigned int)ActiveFields::PauseFeatureUpdates)
                jsonObject->Insert("pauseFeatureUpdates", JsonValue::CreateNumberValue(pauseFeatureUpdates));

            if (activeFields & (unsigned int)ActiveFields::PauseQualityUpdates)
                jsonObject->Insert("pauseQualityUpdates", JsonValue::CreateNumberValue(pauseQualityUpdates));

            if (activeFields & (unsigned int)ActiveFields::ScheduledInstallDay)
                jsonObject->Insert("scheduledInstallDay", JsonValue::CreateNumberValue(scheduledInstallDay));

            if (activeFields & (unsigned int)ActiveFields::ScheduledInstallTime)
                jsonObject->Insert("scheduledInstallTime", JsonValue::CreateNumberValue(scheduledInstallTime));

            if (activeFields & (unsigned int)ActiveFields::Ring)
                jsonObject->Insert("ring", JsonValue::CreateStringValue(ring));

            return jsonObject;
        }

        static WindowsUpdatePolicyConfiguration^ FromJsonObject(JsonObject^ jsonObject)
        {
            auto configuration = ref new WindowsUpdatePolicyConfiguration();

            IJsonValue^ jValue;

            if (jsonObject->HasKey("activeHoursStart"))
            {
                jValue = jsonObject->Lookup("activeHoursStart");
                configuration->activeHoursStart = static_cast<unsigned int>(jValue->GetNumber());
                configuration->activeFields |= (unsigned int)ActiveFields::ActiveHoursStart;
            }

            if (jsonObject->HasKey("activeHoursEnd"))
            {
                jValue = jsonObject->Lookup("activeHoursEnd");
                configuration->activeHoursEnd = static_cast<unsigned int>(jValue->GetNumber());
                configuration->activeFields |= (unsigned int)ActiveFields::ActiveHoursEnd;
            }

            if (jsonObject->HasKey("allowAutoUpdate"))
            {
                jValue = jsonObject->Lookup("allowAutoUpdate");
                configuration->allowAutoUpdate = static_cast<unsigned int>(jValue->GetNumber());
                configuration->activeFields |= (unsigned int)ActiveFields::AllowAutoUpdate;
            }

            if (jsonObject->HasKey("allowUpdateService"))
            {
                jValue = jsonObject->Lookup("allowUpdateService");
                configuration->allowUpdateService = static_cast<unsigned int>(jValue->GetNumber());
                configuration->activeFields |= (unsigned int)ActiveFields::AllowUpdateService;
            }

            if (jsonObject->HasKey("branchReadinessLevel"))
            {
                jValue = jsonObject->Lookup("branchReadinessLevel");
                configuration->branchReadinessLevel = static_cast<unsigned int>(jValue->GetNumber());
                configuration->activeFields |= (unsigned int)ActiveFields::BranchReadinessLevel;
            }

            if (jsonObject->HasKey("deferFeatureUpdatesPeriod"))
            {
                jValue = jsonObject->Lookup("deferFeatureUpdatesPeriod");
                configuration->deferFeatureUpdatesPeriod = static_cast<unsigned int>(jValue->GetNumber());
                configuration->activeFields |= (unsigned int)ActiveFields::DeferFeatureUpdatesPeriod;
            }

            if (jsonObject->HasKey("deferQualityUpdatesPeriod"))
            {
                jValue = jsonObject->Lookup("deferQualityUpdatesPeriod");
                configuration->deferQualityUpdatesPeriod = static_cast<unsigned int>(jValue->GetNumber());
                configuration->activeFields |= (unsigned int)ActiveFields::DeferQualityUpdatesPeriod;
            }

            if (jsonObject->HasKey("pauseFeatureUpdates"))
            {
                jValue = jsonObject->Lookup("pauseFeatureUpdates");
                configuration->pauseFeatureUpdates = static_cast<unsigned int>(jValue->GetNumber());
                configuration->activeFields |= (unsigned int)ActiveFields::PauseFeatureUpdates;
            }

            if (jsonObject->HasKey("pauseQualityUpdates"))
            {
                jValue = jsonObject->Lookup("pauseQualityUpdates");
                configuration->pauseQualityUpdates = static_cast<unsigned int>(jValue->GetNumber());
                configuration->activeFields |= (unsigned int)ActiveFields::PauseQualityUpdates;
            }

            if (jsonObject->HasKey("scheduledInstallDay"))
            {
                jValue = jsonObject->Lookup("scheduledInstallDay");
                configuration->scheduledInstallDay = static_cast<unsigned int>(jValue->GetNumber());
                configuration->activeFields |= (unsigned int)ActiveFields::ScheduledInstallDay;
            }

            if (jsonObject->HasKey("scheduledInstallTime"))
            {
                jValue = jsonObject->Lookup("scheduledInstallTime");
                configuration->scheduledInstallTime = static_cast<unsigned int>(jValue->GetNumber());
                configuration->activeFields |= (unsigned int)ActiveFields::ScheduledInstallTime;
            }

            if (jsonObject->HasKey("ring"))
            {
                jValue = jsonObject->Lookup("ring");
                configuration->ring = jValue->GetString();
                configuration->activeFields |= (unsigned int)ActiveFields::Ring;
            }

            return configuration;
        }
    };

    [Windows::Foundation::Metadata::WebHostHidden]
    public ref class SetWindowsUpdatePolicyRequest sealed : public IRequest
    {
    public:
        property WindowsUpdatePolicyConfiguration^ data;
        property String^ ReportToDeviceTwin;
        property String^ ApplyFromDeviceTwin;

        SetWindowsUpdatePolicyRequest()
        {}

        SetWindowsUpdatePolicyRequest(WindowsUpdatePolicyConfiguration^ d)
        {
            data = d;
        }

        virtual Blob^ Serialize()
        {
            return DeviceTwinDesiredConfiguration<SetWindowsUpdatePolicyRequest>::Serialize(this, (uint32_t)Tag, [](JsonObject^ applyPropertiesObject, SetWindowsUpdatePolicyRequest^ configObject)
            {
                configObject->data->ToJsonObject(applyPropertiesObject);
            });
        }

        static SetWindowsUpdatePolicyRequest^ Deserialize(Platform::String^ str)
        {
            return DeviceTwinDesiredConfiguration<SetWindowsUpdatePolicyRequest>::Deserialize(str, [](JsonObject^ applyPropertiesObject, SetWindowsUpdatePolicyRequest^ configObject)
            {
                configObject->data = WindowsUpdatePolicyConfiguration::FromJsonObject(applyPropertiesObject);
            });
        }

        static IDataPayload^ Deserialize(Blob^ blob)
        {
            assert(blob->Tag == DMMessageKind::SetWindowsUpdatePolicy);
            return Deserialize(SerializationHelper::GetStringFromBlob(blob));
        }

        virtual property DMMessageKind Tag
        {
            DMMessageKind get();
        }
    };

    [Windows::Foundation::Metadata::WebHostHidden]
    public ref class GetWindowsUpdatePolicyRequest sealed : public IRequest
    {
    public:
        GetWindowsUpdatePolicyRequest() {}

        virtual Blob^ Serialize()
        {
            return SerializationHelper::CreateEmptyBlob((uint32_t)Tag);
        }

        static IDataPayload^ Deserialize(Blob^ blob)
        {
            assert(blob->Tag == DMMessageKind::GetWindowsUpdatePolicy);
            return ref new GetWindowsUpdatePolicyRequest();
        }

        virtual property DMMessageKind Tag
        {
            DMMessageKind get();
        }
    };

    [Windows::Foundation::Metadata::WebHostHidden]
    public ref class GetWindowsUpdatePolicyResponse sealed : public IResponse
    {
        StatusCodeResponse statusCodeResponse;
    public:
        property WindowsUpdatePolicyConfiguration^ data;
        property String^ ReportToDeviceTwin;

        GetWindowsUpdatePolicyResponse() :
            statusCodeResponse(ResponseStatus::Success, this->Tag)
        {}

        GetWindowsUpdatePolicyResponse(ResponseStatus status, WindowsUpdatePolicyConfiguration^ d) : statusCodeResponse(status, this->Tag)
        {
            data = d;
        }

        virtual Blob^ Serialize()
        {
            return DeviceTwinReportedConfiguration<GetWindowsUpdatePolicyResponse>::Serialize(this, (uint32_t)Tag, [](JsonObject^ reportPropertiesObject, GetWindowsUpdatePolicyResponse^ configObject)
            {
                configObject->data->ToJsonObject(reportPropertiesObject);
            });
        }

        static GetWindowsUpdatePolicyResponse^ Deserialize(Platform::String^ str)
        {
            return DeviceTwinReportedConfiguration<GetWindowsUpdatePolicyResponse>::Deserialize(str, [](JsonObject^ reportPropertiesObject, GetWindowsUpdatePolicyResponse^ configObject)
            {
                configObject->data = WindowsUpdatePolicyConfiguration::FromJsonObject(reportPropertiesObject);
            });
        }

        static IDataPayload^ Deserialize(Blob^ blob)
        {
            return Deserialize(SerializationHelper::GetStringFromBlob(blob));
        }

        virtual property ResponseStatus Status
        {
            ResponseStatus get() { return statusCodeResponse.Status; }
        }

        virtual property DMMessageKind Tag
        {
            DMMessageKind get();
        }
    };
}
}}}