#pragma once
#include "IRequestIResponse.h"
#include "SerializationHelper.h"
#include "DMMessageKind.h"
#include "StatusCodeResponse.h"
#include "Blob.h"

#include <collection.h>

using namespace Platform;
using namespace Platform::Metadata;
using namespace Platform::Collections;
using namespace Windows::Data::Json;
using namespace Windows::Foundation::Collections;

namespace Microsoft { namespace Devices { namespace Management { namespace Message
{
    public ref class TpmGetServiceUrlRequest sealed : public IRequest
    {
        uint32_t logicalDeviceId;
    public:

        TpmGetServiceUrlRequest(uint32_t logicalDeviceId) : logicalDeviceId(logicalDeviceId) {}

        virtual Blob^ Serialize() {
            JsonObject^ jsonObject = ref new JsonObject();
            jsonObject->Insert("LogicalDeviceId", JsonValue::CreateNumberValue(logicalDeviceId));
            return SerializationHelper::CreateBlobFromJson((uint32_t)Tag, jsonObject);
        }

        static IDataPayload^ Deserialize(Blob^ blob) {
            String^ str = SerializationHelper::GetStringFromBlob(blob);
            JsonObject^ jsonObject = JsonObject::Parse(str);
            uint32_t logicalDeviceId = (uint32_t)jsonObject->Lookup("LogicalDeviceId")->GetNumber();
            return ref new TpmGetServiceUrlRequest(logicalDeviceId);
        }

        virtual property DMMessageKind Tag {
            DMMessageKind get();
        }

        property int LogicalDeviceId {
            int get() { return logicalDeviceId; }
        }
    };

    public ref class TpmGetSASTokenRequest sealed : public IRequest
    {
        uint32_t logicalDeviceId;
        uint32_t expiration;
    public:
        TpmGetSASTokenRequest(uint32_t logicalDeviceId, uint32_t expiration) : logicalDeviceId(logicalDeviceId), expiration(expiration) {}

        virtual Blob^ Serialize() {
            JsonObject^ jsonObject = ref new JsonObject();
            jsonObject->Insert("LogicalDeviceId", JsonValue::CreateNumberValue(logicalDeviceId));
            jsonObject->Insert("Expiration", JsonValue::CreateNumberValue(expiration));
            return SerializationHelper::CreateBlobFromJson((uint32_t)Tag, jsonObject);
        }

        static IDataPayload^ Deserialize(Blob^ blob) {
            String^ str = SerializationHelper::GetStringFromBlob(blob);
            JsonObject^ jsonObject = JsonObject::Parse(str);
            uint32_t logicalDeviceId = (uint32_t)jsonObject->Lookup("LogicalDeviceId")->GetNumber();
            uint32_t expiration = (uint32_t)jsonObject->Lookup("Expiration")->GetNumber();
            return ref new TpmGetSASTokenRequest(logicalDeviceId, expiration);
        }

        virtual property DMMessageKind Tag {
            DMMessageKind get();
        }

        property int LogicalDeviceId {
            int get() { return logicalDeviceId; }
        }

        property int Expiration {
            int get() { return expiration; }
        }
    };

}}}}
