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

    public ref class WindowsUpdatePolicyConfiguration sealed
    {
    public:

        property unsigned int activeFields;

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

        Blob^ Serialize(uint32_t tag) {

            JsonObject^ jsonObject = ref new JsonObject();

            jsonObject->Insert("activeFields", JsonValue::CreateNumberValue(activeFields));

            jsonObject->Insert("activeHoursStart", JsonValue::CreateNumberValue(activeHoursStart));
            jsonObject->Insert("activeHoursEnd", JsonValue::CreateNumberValue(activeHoursEnd));
            jsonObject->Insert("allowAutoUpdate", JsonValue::CreateNumberValue(allowAutoUpdate));
            jsonObject->Insert("allowUpdateService", JsonValue::CreateNumberValue(allowUpdateService));
            jsonObject->Insert("branchReadinessLevel", JsonValue::CreateNumberValue(branchReadinessLevel));

            jsonObject->Insert("deferFeatureUpdatesPeriod", JsonValue::CreateNumberValue(deferFeatureUpdatesPeriod));
            jsonObject->Insert("deferQualityUpdatesPeriod", JsonValue::CreateNumberValue(deferQualityUpdatesPeriod));
            jsonObject->Insert("pauseFeatureUpdates", JsonValue::CreateNumberValue(pauseFeatureUpdates));
            jsonObject->Insert("pauseQualityUpdates", JsonValue::CreateNumberValue(pauseQualityUpdates));
            jsonObject->Insert("scheduledInstallDay", JsonValue::CreateNumberValue(scheduledInstallDay));

            jsonObject->Insert("scheduledInstallTime", JsonValue::CreateNumberValue(scheduledInstallTime));

            jsonObject->Insert("ring", JsonValue::CreateStringValue(ring));

            return SerializationHelper::CreateBlobFromJson(tag, jsonObject);
        }

        static WindowsUpdatePolicyConfiguration^ Deserialize(Blob^ blob) {

            String^ str = SerializationHelper::GetStringFromBlob(blob);
            JsonObject^ jsonObject = JsonObject::Parse(str);

            auto configuration = ref new WindowsUpdatePolicyConfiguration();

            configuration->activeFields = static_cast<unsigned int>(jsonObject->GetNamedNumber("activeFields"));

            configuration->activeHoursStart = static_cast<unsigned int>(jsonObject->GetNamedNumber("activeHoursStart"));
            configuration->activeHoursEnd = static_cast<unsigned int>(jsonObject->GetNamedNumber("activeHoursEnd"));
            configuration->allowAutoUpdate = static_cast<unsigned int>(jsonObject->GetNamedNumber("allowAutoUpdate"));
            configuration->allowUpdateService = static_cast<unsigned int>(jsonObject->GetNamedNumber("allowUpdateService"));
            configuration->branchReadinessLevel = static_cast<unsigned int>(jsonObject->GetNamedNumber("branchReadinessLevel"));

            configuration->deferFeatureUpdatesPeriod = static_cast<unsigned int>(jsonObject->GetNamedNumber("deferFeatureUpdatesPeriod"));
            configuration->deferQualityUpdatesPeriod = static_cast<unsigned int>(jsonObject->GetNamedNumber("deferQualityUpdatesPeriod"));
            configuration->pauseFeatureUpdates = static_cast<unsigned int>(jsonObject->GetNamedNumber("pauseFeatureUpdates"));
            configuration->pauseQualityUpdates = static_cast<unsigned int>(jsonObject->GetNamedNumber("pauseQualityUpdates"));
            configuration->scheduledInstallDay = static_cast<unsigned int>(jsonObject->GetNamedNumber("scheduledInstallDay"));

            configuration->scheduledInstallTime = static_cast<unsigned int>(jsonObject->GetNamedNumber("scheduledInstallTime"));

            configuration->ring = jsonObject->Lookup("ring")->GetString();

            return configuration;
        }
    };

    public ref class SetWindowsUpdatePolicyRequest sealed : public IRequest
    {
    public:
        property WindowsUpdatePolicyConfiguration^ configuration;

        SetWindowsUpdatePolicyRequest(WindowsUpdatePolicyConfiguration^ windowsUpdatePolicyConfiguration)
        {
            configuration = windowsUpdatePolicyConfiguration;
        }

        virtual Blob^ Serialize()
        {
            return configuration->Serialize((uint32_t)Tag);
        }

        static IDataPayload^ Deserialize(Blob^ blob)
        {
            WindowsUpdatePolicyConfiguration^ policyCconfiguration = WindowsUpdatePolicyConfiguration::Deserialize(blob);
            return ref new SetWindowsUpdatePolicyRequest(policyCconfiguration);
        }

        virtual property DMMessageKind Tag {
            DMMessageKind get();
        }
    };

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

    public ref class GetWindowsUpdatePolicyResponse sealed : public IResponse
    {
        StatusCodeResponse statusCodeResponse;
    public:
        property WindowsUpdatePolicyConfiguration^ configuration;

        GetWindowsUpdatePolicyResponse(ResponseStatus status, WindowsUpdatePolicyConfiguration^ updateConfiguration) : statusCodeResponse(status, this->Tag)
        {
            configuration = updateConfiguration;
        }

        virtual Blob^ Serialize()
        {
            return configuration->Serialize((uint32_t)Tag);
        }

        static IDataPayload^ Deserialize(Blob^ blob)
        {
            WindowsUpdatePolicyConfiguration^ policyConfiguration = WindowsUpdatePolicyConfiguration::Deserialize(blob);
            return ref new GetWindowsUpdatePolicyResponse(ResponseStatus::Success, policyConfiguration);
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