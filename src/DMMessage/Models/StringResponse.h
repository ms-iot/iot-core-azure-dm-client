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
