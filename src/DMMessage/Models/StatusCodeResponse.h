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
