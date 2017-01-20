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
    public ref class AppInstallRequest sealed : public IRequest
    {
        String^ appName;
    public:
        AppInstallRequest(String^ appName) : appName(appName) {}

        virtual Blob^ Serialize() {
            JsonObject^ jsonObject = ref new JsonObject();
            jsonObject->Insert("AppName", JsonValue::CreateStringValue(appName));
            return SerializationHelper::CreateBlobFromJson((uint32_t)Tag, jsonObject);
        }

        static AppInstallRequest^ Deserialize(Blob^ bytes) {
            String^ str = SerializationHelper::GetStringFromBlob(bytes);
            JsonObject^ jsonObject = JsonObject::Parse(str);
            auto appName = jsonObject->Lookup("AppName")->GetString();
            return ref new AppInstallRequest(appName);
        }

        virtual property DMMessageKind Tag {
            DMMessageKind get() { return DMMessageKind::InstallApp; }
        }

        property String^ AppName {
            String^ get() { return appName; }
        }
    };

    public ref class AppInstallResponse sealed : public IResponse
    {
        StatusCodeResponse statusCodeResponse;
    public:
        AppInstallResponse(ResponseStatus status) : statusCodeResponse(status, this->Tag) {}
        virtual Blob^ Serialize() {
            return statusCodeResponse.Serialize();
        }

        static AppInstallResponse^ Deserialize(Blob^ bytes) {
            return ref new AppInstallResponse(StatusCodeResponse::Deserialize(bytes)->Status);
        }

        virtual property ResponseStatus Status {
            ResponseStatus get() { return statusCodeResponse.Status; }
        }

        virtual property DMMessageKind Tag {
            DMMessageKind get() { return DMMessageKind::InstallApp; }
        }
    };
}
}}}
