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
    public ref class AppUninstallInfo sealed
    {
    private:
        String^ _PackageFamilyName;
        bool _StoreApp;
    public:
        AppUninstallInfo(String^ appId, bool start)
        {
            _PackageFamilyName = appId;
            _StoreApp = start;
        }
        property String^ PackageFamilyName { String^ get() { return _PackageFamilyName; }; }
        property bool StoreApp { bool get() { return _StoreApp; }; }
    };

    public ref class AppUninstallRequest sealed : public IRequest
    {
        AppUninstallInfo^ appInfo;
    public:
        AppUninstallRequest(AppUninstallInfo^ appInfo) : appInfo(appInfo) {}

        virtual Blob^ Serialize() {
            JsonObject^ jsonObject = ref new JsonObject();
            jsonObject->Insert("PackageFamilyName", JsonValue::CreateStringValue(appInfo->PackageFamilyName));
            jsonObject->Insert("StoreApp", JsonValue::CreateBooleanValue(appInfo->StoreApp));
            return SerializationHelper::CreateBlobFromJson((uint32_t)Tag, jsonObject);
        }

        static IDataPayload^ Deserialize(Blob^ bytes) {
            String^ str = SerializationHelper::GetStringFromBlob(bytes);
            JsonObject^ jsonObject = JsonObject::Parse(str);
            auto appId = jsonObject->Lookup("PackageFamilyName")->GetString();
            auto start = jsonObject->Lookup("StoreApp")->GetBoolean();
            auto appInfo = ref new Microsoft::Devices::Management::Message::AppUninstallInfo(appId, start);
            return ref new AppUninstallRequest(appInfo);
        }

        virtual property DMMessageKind Tag {
            DMMessageKind get();
        }

        property AppUninstallInfo^ AppUninstallInfo {
            Microsoft::Devices::Management::Message::AppUninstallInfo^ get() { return appInfo; }
        }
    };

    public ref class AppUninstallResponse sealed : public IResponse
    {
        StatusCodeResponse statusCodeResponse;
    public:
        AppUninstallResponse(ResponseStatus status) : statusCodeResponse(status, this->Tag) {}
        virtual Blob^ Serialize() {
            return statusCodeResponse.Serialize();
        }

        static IDataPayload^ Deserialize(Blob^ bytes) {
            return ref new AppUninstallResponse(StatusCodeResponse::Deserialize(bytes)->Status);
        }

        virtual property ResponseStatus Status {
            ResponseStatus get() { return statusCodeResponse.Status; }
        }

        virtual property DMMessageKind Tag {
            DMMessageKind get();
        }
    };
}}}}
