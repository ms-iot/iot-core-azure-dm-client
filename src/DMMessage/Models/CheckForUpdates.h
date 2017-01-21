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
    public ref class CheckForUpdatesRequest sealed : public IRequest
    {
    public:
        virtual Blob^ Serialize() {
            return SerializationHelper::CreateEmptyBlob((uint32_t)Tag);
        }

        static CheckForUpdatesRequest^ Deserialize(Blob^ blob) {
            assert(blob->Tag == DMMessageKind::CheckUpdates);
            return ref new CheckForUpdatesRequest();
        }

        virtual property DMMessageKind Tag {
            DMMessageKind get() { return DMMessageKind::CheckUpdates; }
        }
    };

    public ref class CheckForUpdatesResponse sealed : public IResponse
    {
        ResponseStatus status;
        bool updatesAvailable;
    public:
        CheckForUpdatesResponse(ResponseStatus status, bool updatesAvailable) : status(status), updatesAvailable(updatesAvailable) {}

        virtual Blob^ Serialize() {
            JsonObject^ jsonObject = ref new JsonObject();
            jsonObject->Insert("Status", JsonValue::CreateNumberValue((uint32_t)status));
            jsonObject->Insert("UpdatesAvailable", JsonValue::CreateNumberValue((uint32_t)updatesAvailable));
            return SerializationHelper::CreateBlobFromJson((uint32_t)Tag, jsonObject);
        }

        static CheckForUpdatesResponse^ Deserialize(Blob^ bytes) {
            String^ str = SerializationHelper::GetStringFromBlob(bytes);
            JsonObject^ jsonObject = JsonObject::Parse(str);
            ResponseStatus status = (ResponseStatus)(uint32_t)jsonObject->Lookup("Status")->GetNumber();
            bool updatesAvailable = !!(uint32_t)jsonObject->Lookup("UpdatesAvailable")->GetNumber();
            return ref new CheckForUpdatesResponse(status, updatesAvailable);
        }

        virtual property DMMessageKind Tag {
            DMMessageKind get() { return DMMessageKind::CheckUpdates; }
        }

        virtual property ResponseStatus Status {
            ResponseStatus get() { return status; }
        }

        property bool UpdatesAvailable {
            bool get() { return updatesAvailable; }
        }
    };
}
}}}
