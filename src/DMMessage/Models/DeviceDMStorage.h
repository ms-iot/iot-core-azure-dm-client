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
    public ref class GetDMFoldersRequest sealed : public IRequest
    {
    public:
        virtual Blob^ Serialize() {
            return SerializationHelper::CreateEmptyBlob((uint32_t)Tag);
        }

        static IDataPayload^ Deserialize(Blob^ bytes) {
            return ref new GetDMFoldersRequest();
        }

        virtual property DMMessageKind Tag {
            DMMessageKind get();
        }
    };

    public ref class GetDMFilesRequest sealed : public IRequest
    {
    public:
        property String^ DMFolderName;

        virtual Blob^ Serialize()
        {
            JsonObject^ jsonObject = ref new JsonObject();
            jsonObject->Insert("folderName", JsonValue::CreateStringValue(DMFolderName));
            return SerializationHelper::CreateBlobFromJson((uint32_t)Tag, jsonObject);
        }

        static IDataPayload^ Deserialize(Blob^ blob)
        {
            String^ str = SerializationHelper::GetStringFromBlob(blob);
            JsonObject^ jsonObject = JsonObject::Parse(str);
            auto result = ref new GetDMFilesRequest();
            result->DMFolderName = jsonObject->Lookup("folderName")->GetString();
            return result;
        }

        virtual property DMMessageKind Tag
        {
            DMMessageKind get();
        }
    };

    public ref class DeleteDMFileRequest sealed : public IRequest
    {
    public:
        property String^ DMFolderName;
        property String^ DMFileName;

        virtual Blob^ Serialize()
        {

            JsonObject^ jsonObject = ref new JsonObject();
            jsonObject->Insert("folderName", JsonValue::CreateStringValue(DMFolderName));
            jsonObject->Insert("fileName", JsonValue::CreateStringValue(DMFileName));
            return SerializationHelper::CreateBlobFromJson((uint32_t)Tag, jsonObject);
        }

        static IDataPayload^ Deserialize(Blob^ blob)
        {
            String^ str = SerializationHelper::GetStringFromBlob(blob);
            JsonObject^ jsonObject = JsonObject::Parse(str);
            auto result = ref new DeleteDMFileRequest();
            result->DMFolderName = jsonObject->Lookup("folderName")->GetString();
            result->DMFileName = jsonObject->Lookup("fileName")->GetString();
            return result;
        }

        virtual property DMMessageKind Tag
        {
            DMMessageKind get();
        }
    };
}
}}}
