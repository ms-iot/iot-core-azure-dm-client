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
    public ref class WindowsUpdatePolicyConfiguration sealed
    {
    public:
        property unsigned int activeHoursStart;
        property unsigned int activeHoursEnd;
        property unsigned int allowAutoUpdate;
        property unsigned int allowMUUpdateService;
        property unsigned int allowNonMicrosoftSignedUpdate;

        property unsigned int allowUpdateService;
        property unsigned int branchReadinessLevel;
        property unsigned int deferFeatureUpdatesPeriod;    // in days
        property unsigned int deferQualityUpdatesPeriod;    // in days
        property unsigned int excludeWUDrivers;

        property unsigned int pauseFeatureUpdates;
        property unsigned int pauseQualityUpdates;
        property unsigned int requireUpdateApproval;
        property unsigned int scheduledInstallDay;
        property unsigned int scheduledInstallTime;

        property String^ updateServiceUrl;

        WindowsUpdatePolicyConfiguration()
        {
            activeHoursStart = 0;
            activeHoursEnd = 0;
            allowAutoUpdate = 0;
            allowMUUpdateService = 0;
            allowNonMicrosoftSignedUpdate = 0;

            allowUpdateService = 0;
            branchReadinessLevel = 0;
            deferFeatureUpdatesPeriod = 0;    // in days
            deferQualityUpdatesPeriod = 0;    // in days
            excludeWUDrivers = 0;

            pauseFeatureUpdates = 0;
            pauseQualityUpdates = 0;
            requireUpdateApproval = 0;
            scheduledInstallDay = 0;
            scheduledInstallTime = 0;

            updateServiceUrl = ref new String();
        }

        Blob^ Serialize(uint32_t tag) {

            JsonObject^ jsonObject = ref new JsonObject();

            jsonObject->Insert("activeHoursStart", JsonValue::CreateNumberValue(activeHoursStart));
            jsonObject->Insert("activeHoursEnd", JsonValue::CreateNumberValue(activeHoursEnd));
            jsonObject->Insert("allowAutoUpdate", JsonValue::CreateNumberValue(allowAutoUpdate));
            jsonObject->Insert("allowMUUpdateService", JsonValue::CreateNumberValue(allowMUUpdateService));
            jsonObject->Insert("allowNonMicrosoftSignedUpdate", JsonValue::CreateNumberValue(allowNonMicrosoftSignedUpdate));

            jsonObject->Insert("allowUpdateService", JsonValue::CreateNumberValue(allowUpdateService));
            jsonObject->Insert("branchReadinessLevel", JsonValue::CreateNumberValue(branchReadinessLevel));
            jsonObject->Insert("deferFeatureUpdatesPeriod", JsonValue::CreateNumberValue(deferFeatureUpdatesPeriod));
            jsonObject->Insert("deferQualityUpdatesPeriod", JsonValue::CreateNumberValue(deferQualityUpdatesPeriod));
            jsonObject->Insert("excludeWUDrivers", JsonValue::CreateNumberValue(excludeWUDrivers));

            jsonObject->Insert("pauseFeatureUpdates", JsonValue::CreateNumberValue(pauseFeatureUpdates));
            jsonObject->Insert("pauseQualityUpdates", JsonValue::CreateNumberValue(pauseQualityUpdates));
            jsonObject->Insert("requireUpdateApproval", JsonValue::CreateNumberValue(requireUpdateApproval));
            jsonObject->Insert("scheduledInstallDay", JsonValue::CreateNumberValue(scheduledInstallDay));
            jsonObject->Insert("scheduledInstallTime", JsonValue::CreateNumberValue(scheduledInstallTime));

            jsonObject->Insert("updateServiceUrl", JsonValue::CreateStringValue(updateServiceUrl));

            return SerializationHelper::CreateBlobFromJson(tag, jsonObject);
        }

        static WindowsUpdatePolicyConfiguration^ Deserialize(Blob^ blob) {

            String^ str = SerializationHelper::GetStringFromBlob(blob);
            JsonObject^ jsonObject = JsonObject::Parse(str);

            auto configuration = ref new WindowsUpdatePolicyConfiguration();
            configuration->activeHoursStart = static_cast<unsigned int>(jsonObject->GetNamedNumber("activeHoursStart"));
            configuration->activeHoursEnd = static_cast<unsigned int>(jsonObject->GetNamedNumber("activeHoursEnd"));
            configuration->allowAutoUpdate = static_cast<unsigned int>(jsonObject->GetNamedNumber("allowAutoUpdate"));
            configuration->allowMUUpdateService = static_cast<unsigned int>(jsonObject->GetNamedNumber("allowMUUpdateService"));
            configuration->allowNonMicrosoftSignedUpdate = static_cast<unsigned int>(jsonObject->GetNamedNumber("allowNonMicrosoftSignedUpdate"));

            configuration->allowUpdateService = static_cast<unsigned int>(jsonObject->GetNamedNumber("allowUpdateService"));
            configuration->branchReadinessLevel = static_cast<unsigned int>(jsonObject->GetNamedNumber("branchReadinessLevel"));
            configuration->deferFeatureUpdatesPeriod = static_cast<unsigned int>(jsonObject->GetNamedNumber("deferFeatureUpdatesPeriod"));
            configuration->deferQualityUpdatesPeriod = static_cast<unsigned int>(jsonObject->GetNamedNumber("deferQualityUpdatesPeriod"));
            configuration->excludeWUDrivers = static_cast<unsigned int>(jsonObject->GetNamedNumber("excludeWUDrivers"));

            configuration->pauseFeatureUpdates = static_cast<unsigned int>(jsonObject->GetNamedNumber("pauseFeatureUpdates"));
            configuration->pauseQualityUpdates = static_cast<unsigned int>(jsonObject->GetNamedNumber("pauseQualityUpdates"));
            configuration->requireUpdateApproval = static_cast<unsigned int>(jsonObject->GetNamedNumber("requireUpdateApproval"));
            configuration->scheduledInstallDay = static_cast<unsigned int>(jsonObject->GetNamedNumber("scheduledInstallDay"));
            configuration->scheduledInstallTime = static_cast<unsigned int>(jsonObject->GetNamedNumber("scheduledInstallTime"));

            configuration->updateServiceUrl = jsonObject->Lookup("updateServiceUrl")->GetString();

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

