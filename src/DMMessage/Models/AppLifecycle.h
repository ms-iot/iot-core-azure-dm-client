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
    public ref class AppLifecycleInfo sealed
    {
    private:
        String^ _AppId;
        bool _Start;
    public:
        AppLifecycleInfo(String^ appId, bool start)
        {
            _AppId = appId;
            _Start = start;
        }
        property String^ AppId { String^ get() { return _AppId; }; }
        property bool Start { bool get() { return _Start; }; }
    };

    public ref class AppLifecycleRequest sealed : public IRequest
    {
    private:
        AppLifecycleInfo^ appInfo;
    
    public:
        AppLifecycleRequest(AppLifecycleInfo^ appInfo) : appInfo(appInfo) {}

        virtual Blob^ Serialize() {
            JsonObject^ jsonObject = ref new JsonObject();
            jsonObject->Insert("AppId", JsonValue::CreateStringValue(appInfo->AppId));
            jsonObject->Insert("Start", JsonValue::CreateBooleanValue(appInfo->Start));
            return SerializationHelper::CreateBlobFromJson((uint32_t)Tag, jsonObject);
        }

        static IDataPayload^ Deserialize(Blob^ bytes) {
            String^ str = SerializationHelper::GetStringFromBlob(bytes);
            JsonObject^ jsonObject = JsonObject::Parse(str);
            auto appId = jsonObject->Lookup("AppId")->GetString();
            auto start = jsonObject->Lookup("Start")->GetBoolean();
            auto appInfo = ref new Microsoft::Devices::Management::Message::AppLifecycleInfo(appId, start);
            return ref new AppLifecycleRequest(appInfo);
        }

        virtual property DMMessageKind Tag {
            DMMessageKind get() { return (appInfo->Start) ? DMMessageKind::StartApp : DMMessageKind::StopApp; };
        }

        property AppLifecycleInfo^ AppLifecycleInfo {
            Microsoft::Devices::Management::Message::AppLifecycleInfo^ get() { return appInfo; }
        }
    };
}}}}
