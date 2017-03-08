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
    public ref class WindowsUpdateRebootPolicyConfiguration sealed
    {
    public:
        property bool allow;

        WindowsUpdateRebootPolicyConfiguration()
        {
            allow = true;
        }

        Blob^ Serialize(uint32_t tag)
        {
            JsonObject^ jsonObject = ref new JsonObject();
            jsonObject->Insert("allow", JsonValue::CreateBooleanValue(allow));
            return SerializationHelper::CreateBlobFromJson(tag, jsonObject);
        }

        static WindowsUpdateRebootPolicyConfiguration^ Deserialize(Blob^ blob)
        {
            String^ str = SerializationHelper::GetStringFromBlob(blob);
            JsonObject^ jsonObject = JsonObject::Parse(str);

            auto configuration = ref new WindowsUpdateRebootPolicyConfiguration();
            configuration->allow = jsonObject->Lookup("allow")->GetBoolean();
            return configuration;
        }
    };

    public ref class SetWindowsUpdateRebootPolicyRequest sealed : public IRequest
    {
    public:
        property WindowsUpdateRebootPolicyConfiguration^ configuration;

        SetWindowsUpdateRebootPolicyRequest(WindowsUpdateRebootPolicyConfiguration^ windowsUpdatePolicyConfiguration)
        {
            configuration = windowsUpdatePolicyConfiguration;
        }

        virtual Blob^ Serialize()
        {
            return configuration->Serialize((uint32_t)Tag);
        }

        static IDataPayload^ Deserialize(Blob^ blob)
        {
            WindowsUpdateRebootPolicyConfiguration^ policyConfiguration = WindowsUpdateRebootPolicyConfiguration::Deserialize(blob);
            return ref new SetWindowsUpdateRebootPolicyRequest(policyConfiguration);
        }

        virtual property DMMessageKind Tag {
            DMMessageKind get();
        }
    };

    public ref class GetWindowsUpdateRebootPolicyRequest sealed : public IRequest
    {
    public:
        GetWindowsUpdateRebootPolicyRequest() {}

        virtual Blob^ Serialize()
        {
            return SerializationHelper::CreateEmptyBlob((uint32_t)Tag);
        }

        static IDataPayload^ Deserialize(Blob^ blob)
        {
            assert(blob->Tag == DMMessageKind::GetWindowsUpdateRebootPolicy);
            return ref new GetWindowsUpdateRebootPolicyRequest();
        }

        virtual property DMMessageKind Tag
        {
            DMMessageKind get();
        }
    };

    public ref class GetWindowsUpdateRebootPolicyResponse sealed : public IResponse
    {
        StatusCodeResponse statusCodeResponse;
    public:
        property WindowsUpdateRebootPolicyConfiguration^ configuration;

        GetWindowsUpdateRebootPolicyResponse(ResponseStatus status, WindowsUpdateRebootPolicyConfiguration^ updateConfiguration) : statusCodeResponse(status, this->Tag)
        {
            configuration = updateConfiguration;
        }

        virtual Blob^ Serialize()
        {
            return configuration->Serialize((uint32_t)Tag);
        }

        static IDataPayload^ Deserialize(Blob^ blob)
        {
            auto policyConfiguration = WindowsUpdateRebootPolicyConfiguration::Deserialize(blob);
            return ref new GetWindowsUpdateRebootPolicyResponse(ResponseStatus::Success, policyConfiguration);
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