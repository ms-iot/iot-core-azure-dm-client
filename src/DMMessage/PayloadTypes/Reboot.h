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
    public ref class RebootRequest sealed : public IRequest
    {
    public:
        virtual Blob^ Serialize() {
            return SerializationHelper::CreateEmptyBlob((uint32_t)Tag);
        }

        static RebootRequest^ Deserialize(Blob^ blob) {
            assert(blob->Tag == DMMessageKind::RebootSystem);
            return ref new RebootRequest();
        }

        virtual property DMMessageKind Tag {
            DMMessageKind get() { return DMMessageKind::RebootSystem; }
        }
    };

}}}}
