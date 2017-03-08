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

    public ref class FactoryResetRequest sealed : public IRequest
    {
    public:
        property bool clearTPM;
        property String^ recoveryPartitionGUID;

        virtual Blob^ Serialize() {
            JsonObject^ jsonObject = ref new JsonObject();
            jsonObject->Insert("clearTPM", JsonValue::CreateBooleanValue(clearTPM));
            jsonObject->Insert("recoveryPartitionGUID", JsonValue::CreateStringValue(recoveryPartitionGUID));
            return SerializationHelper::CreateBlobFromJson((uint32_t)Tag, jsonObject);
        }

        static IDataPayload^ Deserialize(Blob^ blob) {
            assert(blob->Tag == DMMessageKind::FactoryReset);
            String^ str = SerializationHelper::GetStringFromBlob(blob);
            JsonObject^ jsonObject = JsonObject::Parse(str);
            auto request = ref new FactoryResetRequest();
            request->clearTPM = jsonObject->Lookup("clearTPM")->GetBoolean();
            request->recoveryPartitionGUID = jsonObject->Lookup("recoveryPartitionGUID")->GetString();
            return request;
        }

        virtual property DMMessageKind Tag {
            DMMessageKind get();
        }
    };

}}}}
