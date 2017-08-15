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
    public ref class AzureFileTransferInfo sealed
    {
    public:
        AzureFileTransferInfo()
        {
            RelativeLocalPath = ref new Platform::String();
            AppLocalDataPath = ref new Platform::String();
            ConnectionString = ref new Platform::String();
            ContainerName = ref new Platform::String();
            BlobName = ref new Platform::String();
            Upload = true;
        }
        AzureFileTransferInfo(String^ relativeLocalPath, String^ appLocalDataPath, String^ connectionString, String^ containerName, String^ blobName, bool upload)
        {
            RelativeLocalPath = relativeLocalPath;
            AppLocalDataPath = appLocalDataPath;
            ConnectionString = connectionString;
            ContainerName = containerName;
            BlobName = blobName;
            Upload = upload;
        }
        property String^ RelativeLocalPath;
        property String^ AppLocalDataPath;
        property String^ ConnectionString;
        property String^ ContainerName;
        property String^ BlobName;
        property bool Upload;
    };

    public ref class AzureFileTransferRequest sealed : public IRequest
    {
        AzureFileTransferInfo^ appInfo;
    public:
        AzureFileTransferRequest(AzureFileTransferInfo^ appInfo) : appInfo(appInfo) {}

        virtual Blob^ Serialize() {
            JsonObject^ jsonObject = ref new JsonObject();
            jsonObject->Insert("RelativeLocalPath", JsonValue::CreateStringValue(appInfo->RelativeLocalPath));
            jsonObject->Insert("AppLocalDataPath", JsonValue::CreateStringValue(appInfo->AppLocalDataPath));
            jsonObject->Insert("ConnectionString", JsonValue::CreateStringValue(appInfo->ConnectionString));
            jsonObject->Insert("ContainerName", JsonValue::CreateStringValue(appInfo->ContainerName));
            jsonObject->Insert("BlobName", JsonValue::CreateStringValue(appInfo->BlobName));
            jsonObject->Insert("Upload", JsonValue::CreateBooleanValue(appInfo->Upload));

            return SerializationHelper::CreateBlobFromJson((uint32_t)Tag, jsonObject);
        }

        static IDataPayload^ Deserialize(Blob^ bytes) {
            String^ str = SerializationHelper::GetStringFromBlob(bytes);
            JsonObject^ jsonObject = JsonObject::Parse(str);
            auto relativeLocalPath = jsonObject->Lookup("RelativeLocalPath")->GetString();
            auto appLocalDataPath = jsonObject->Lookup("AppLocalDataPath")->GetString();
            auto connectionString = jsonObject->Lookup("ConnectionString")->GetString();
            auto containerName = jsonObject->Lookup("ContainerName")->GetString();
            auto blobName = jsonObject->Lookup("BlobName")->GetString();
            auto upload = jsonObject->Lookup("Upload")->GetBoolean();

            auto appInfo = ref new Microsoft::Devices::Management::Message::AzureFileTransferInfo(relativeLocalPath, appLocalDataPath, connectionString, containerName, blobName, upload);
            return ref new AzureFileTransferRequest(appInfo);
        }

        virtual property DMMessageKind Tag {
            DMMessageKind get();
        }

        property AzureFileTransferInfo^ AzureFileTransferInfo {
            Microsoft::Devices::Management::Message::AzureFileTransferInfo^ get() { return appInfo; }
        }
    };
}}}}
