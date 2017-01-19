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
    // Implementation of a simple response class with uint32_t status and no other payload
    // This can be used as a helper class to construct other response classes or independently
    public ref class StatusCodeResponse sealed : public IResponse
    {
        ResponseStatus status;
        DMMessageKind tag;
    public:
        StatusCodeResponse(ResponseStatus status, DMMessageKind alttag) : status(status), tag(alttag) {}
        virtual Blob^ Serialize() {
            return SerializationHelper::CreateBlobFromPtrSize((uint32_t)tag, (const uint8_t*)&status, sizeof(uint32_t));
        }

        static StatusCodeResponse^ Deserialize(Blob^ blob) {
            ResponseStatus status;
            SerializationHelper::ReadDataFromBlob(blob, (uint8_t*)&status, sizeof(status));
            return ref new StatusCodeResponse(status, blob->Tag);
        }

        virtual property DMMessageKind Tag {
            DMMessageKind get() { return tag; }
        }

        virtual property ResponseStatus Status {
            ResponseStatus get() { return status; }
        }
    };
}
}}}
