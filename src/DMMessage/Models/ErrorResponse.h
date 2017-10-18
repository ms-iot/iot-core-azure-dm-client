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
#include "../SharedUtilities/StringUtils.h"

using namespace Platform;
using namespace Platform::Metadata;
using namespace Windows::Data::Json;

namespace Microsoft { namespace Devices { namespace Management { namespace Message
{
    public ref class ErrorResponse sealed : public IResponse
    {
        ResponseStatus status;
        DMMessageKind tag;

    public:

        property ErrorSubSystem SubSystem;
        property int ErrorCode;
        property String^ ErrorMessage;

        ErrorResponse(ErrorSubSystem subSystem, int code, String^ message) :
            status(ResponseStatus::Failure),
            tag(DMMessageKind::ErrorResponse)
        {
            SubSystem = subSystem;
            ErrorCode = code;
            ErrorMessage = message;
        }

        virtual Blob^ Serialize()
        {
            JsonObject^ jsonObject = ref new JsonObject();
            jsonObject->Insert("Status", JsonValue::CreateNumberValue((uint32_t)status));
            jsonObject->Insert("SubSystem", JsonValue::CreateNumberValue((uint32_t)SubSystem));
            jsonObject->Insert("ErrorCode", JsonValue::CreateNumberValue(ErrorCode));
            jsonObject->Insert("ErrorMessage", JsonValue::CreateStringValue(ErrorMessage));
            return SerializationHelper::CreateBlobFromJson((uint32_t)Tag, jsonObject);
        }

        static ErrorResponse^ Deserialize(Blob^ blob)
        {
            String^ str = SerializationHelper::GetStringFromBlob(blob);
            JsonObject^ jsonObject = JsonObject::Parse(str);
            ErrorSubSystem subSystem = ErrorSubSystemConverter::FromDouble(jsonObject->GetNamedNumber("SubSystem"));
            int errorCode = static_cast<int>(jsonObject->GetNamedNumber("ErrorCode"));
            String^ errorMessage = jsonObject->GetNamedString("ErrorMessage");
            return ref new ErrorResponse(subSystem, errorCode, errorMessage);
        }

        virtual property DMMessageKind Tag {
            DMMessageKind get() { return tag; }
        }

        virtual property ResponseStatus Status {
            ResponseStatus get() { return status; }
        }
    };

    static ErrorResponse^ CreateErrorResponse(ErrorSubSystem subSystem, int code, const char* message)
    {
        String^ errorMessage = ref new String(Utils::MultibyteToWide(message).c_str());
        return ref new ErrorResponse(subSystem, code, errorMessage);
    }

}
}}}
