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
        AzureFileTransferInfo(String^ localPath, String^ appLocalDataPath, String^ connectionString, String^ containerName, String^ blobName, bool upload)
        {
            LocalPath = localPath;
            AppLocalDataPath = appLocalDataPath;
            ConnectionString = connectionString;
            ContainerName = containerName;
            BlobName = blobName;
            Upload = upload;
        }
        property String^ LocalPath;
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
            jsonObject->Insert("LocalPath", JsonValue::CreateStringValue(appInfo->LocalPath));
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
            auto localPath = jsonObject->Lookup("LocalPath")->GetString();
            auto appLocalDataPath = jsonObject->Lookup("AppLocalDataPath")->GetString();
            auto connectionString = jsonObject->Lookup("ConnectionString")->GetString();
            auto containerName = jsonObject->Lookup("ContainerName")->GetString();
            auto blobName = jsonObject->Lookup("BlobName")->GetString();
            auto upload = jsonObject->Lookup("Upload")->GetBoolean();

            auto appInfo = ref new Microsoft::Devices::Management::Message::AzureFileTransferInfo(localPath, appLocalDataPath, connectionString, containerName, blobName, upload);
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
