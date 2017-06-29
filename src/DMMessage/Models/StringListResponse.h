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
#include "StatusCodeResponse.h"
#include "Blob.h"

using namespace Platform;
using namespace Platform::Metadata;
using namespace Windows::Data::Json;

namespace Microsoft { namespace Devices { namespace Management { namespace Message
{
    public ref class StringListResponse sealed : public IResponse
    {
        StatusCodeResponse statusCodeResponse;
    public:
        property IVector<String^>^ List;

        StringListResponse(ResponseStatus status) : statusCodeResponse(status, this->Tag)
        {
            List = ref new Vector<String^>();
        }

        virtual Blob^ Serialize()
        {
            JsonArray^ jsonArray = ref new JsonArray();
            for each (String^ folder in List)
            {
                jsonArray->Append(JsonValue::CreateStringValue(folder));
            }

            JsonObject^ jsonObject = ref new JsonObject();
            jsonObject->Insert("list", jsonArray);
            return SerializationHelper::CreateBlobFromJson((uint32_t)Tag, jsonObject);
        }

        static IDataPayload^ Deserialize(Blob^ blob)
        {
            String^ str = SerializationHelper::GetStringFromBlob(blob);
            JsonObject^ jsonObject = JsonObject::Parse(str);
            auto result = ref new StringListResponse(ResponseStatus::Success);

            JsonArray^ jsonArray = jsonObject->Lookup("list")->GetArray();
            for (unsigned int i = 0; i < jsonArray->Size; ++i)
            {
                result->List->Append(jsonArray->GetStringAt(i));
            }
            return result;
        }

        virtual property ResponseStatus Status {
            ResponseStatus get() { return statusCodeResponse.Status; }
        }

        virtual property DMMessageKind Tag {
            DMMessageKind get();
        }
    };
}
}}}
