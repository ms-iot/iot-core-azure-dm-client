#pragma once
#include "IRequestIResponse.h"
#include "SerializationHelper.h"
#include "DMMessageKind.h"
#include "ResponseStatus.h"
#include "Blob.h"

using namespace Platform;
using namespace Platform::Metadata;
using namespace Windows::Data::Json;

namespace Microsoft { namespace Devices { namespace Management { namespace Message
{
    public ref class StringResponse sealed : public IResponse
    {
        ResponseStatus status;
        DMMessageKind tag;
        String^ response;
    public:
        StringResponse(ResponseStatus status, String^ response, DMMessageKind alttag) : status(status), response(response), tag(alttag) {}
        virtual Blob^ Serialize() {
            JsonObject^ jsonObject = ref new JsonObject();
            jsonObject->Insert("Response", JsonValue::CreateStringValue(response));
            jsonObject->Insert("Status", JsonValue::CreateNumberValue((uint32_t)status));
            return SerializationHelper::CreateBlobFromJson((uint32_t)Tag, jsonObject);
        }

        static StringResponse^ Deserialize(Blob^ blob) {
            String^ str = SerializationHelper::GetStringFromBlob(blob);
            JsonObject^ jsonObject = JsonObject::Parse(str);
            auto response = jsonObject->GetNamedString("Response");
            auto status = (ResponseStatus)(uint32_t)jsonObject->GetNamedNumber("Status");
            return ref new StringResponse(status, response, blob->Tag);
        }

        virtual property DMMessageKind Tag {
            DMMessageKind get() { return tag; }
        }

        virtual property ResponseStatus Status {
            ResponseStatus get() { return status; }
        }

        property String^ Response {
            String^ get() { return response; }
        }
    };
}
}}}
