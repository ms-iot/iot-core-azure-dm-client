#pragma once
#include "IRequestIResponse.h"
#include "SerializationHelper.h"
#include "DMMessageKind.h"
#include "StatusCodeResponse.h"
#include "Blob.h"

#include <collection.h>

using namespace Platform;
using namespace Platform::Metadata;
using namespace Platform::Collections;
using namespace Windows::Data::Json;
using namespace Windows::Foundation::Collections;

namespace Microsoft { namespace Devices { namespace Management { namespace Message
{
    public ref class StartupAppInfo sealed
    {
    private:
        String^ _AppId;
        bool _IsBackgroundApplication;
    public:
        StartupAppInfo(String^ appId, bool start)
        {
            _AppId = appId;
            _IsBackgroundApplication = start;
        }
        property String^ AppId { String^ get() { return _AppId; }; }
        property bool IsBackgroundApplication { bool get() { return _IsBackgroundApplication; }; }
    };

    public ref class StartupAppRequest sealed : public IRequest
    {
        StartupAppInfo^ appInfo;
    public:
        StartupAppRequest(StartupAppInfo^ appInfo) : appInfo(appInfo) {}

        virtual Blob^ Serialize() {
            JsonObject^ jsonObject = ref new JsonObject();
            jsonObject->Insert("AppId", JsonValue::CreateStringValue(appInfo->AppId));
            jsonObject->Insert("IsBackgroundApplication", JsonValue::CreateBooleanValue(appInfo->IsBackgroundApplication));
            return SerializationHelper::CreateBlobFromJson((uint32_t)Tag, jsonObject);
        }

        static IDataPayload^ Deserialize(Blob^ bytes) {
            String^ str = SerializationHelper::GetStringFromBlob(bytes);
            JsonObject^ jsonObject = JsonObject::Parse(str);
            auto appId = jsonObject->Lookup("AppId")->GetString();
            auto start = jsonObject->Lookup("IsBackgroundApplication")->GetBoolean();
            auto appInfo = ref new Microsoft::Devices::Management::Message::StartupAppInfo(appId, start);
            return ref new StartupAppRequest(appInfo);
        }

        virtual property DMMessageKind Tag {
            DMMessageKind get();
        }

        property StartupAppInfo^ StartupAppInfo {
            Microsoft::Devices::Management::Message::StartupAppInfo^ get() { return appInfo; }
        }
    };

    public ref class GetStartupForegroundAppResponse sealed : public IResponse
    {
        ResponseStatus status;
        String^ startupForegroundApp;
    public:
        GetStartupForegroundAppResponse(ResponseStatus status, String^ startupForegroundApp) : status(status), startupForegroundApp(startupForegroundApp) {}

        virtual Blob^ Serialize() {
            JsonObject^ jsonObject = ref new JsonObject();
            jsonObject->Insert("Status", JsonValue::CreateNumberValue((uint32_t)status));
            jsonObject->Insert("StartupForegroundApp", JsonValue::CreateStringValue(startupForegroundApp));
            return SerializationHelper::CreateBlobFromJson((uint32_t)Tag, jsonObject);
        }

        static IDataPayload^ Deserialize(Blob^ bytes) {
            auto str = SerializationHelper::GetStringFromBlob(bytes);
            auto jsonObject = JsonObject::Parse(str);
            auto status = (ResponseStatus)(uint32_t)jsonObject->Lookup("Status")->GetNumber();
            auto startupForegroundApp = jsonObject->Lookup("StartupForegroundApp")->GetString();
            return ref new GetStartupForegroundAppResponse(status, startupForegroundApp);
        }

        virtual property DMMessageKind Tag {
            DMMessageKind get();
        }

        virtual property ResponseStatus Status {
            ResponseStatus get() { return status; }
        }

        property String^ StartupForegroundApp {
            String^ get() { return startupForegroundApp; }
        }
    };

    public ref class ListStartupBackgroundAppsResponse sealed : public IResponse
    {
    private:
        ResponseStatus status;
        IVector<String^>^ startupBackgroundApps;

    public:
        ListStartupBackgroundAppsResponse(ResponseStatus status, IVector<String^>^ startupBackgroundApps) : status(status), startupBackgroundApps(startupBackgroundApps) {}

        virtual Blob^ Serialize() {
            auto jsonObject = ref new JsonObject();
            jsonObject->Insert("Status", JsonValue::CreateNumberValue((uint32_t)status));
            auto jsonArray = ref new JsonArray();
            for (unsigned int i=0; i<startupBackgroundApps->Size; i++)
            {
                jsonArray->Append(JsonValue::CreateStringValue(startupBackgroundApps->GetAt(i)));
            }
            jsonObject->Insert("StartupBackgroundApps", jsonArray);
            return SerializationHelper::CreateBlobFromJson((uint32_t)Tag, jsonObject);
        }

        static IDataPayload^ Deserialize(Blob^ bytes) {
            auto str = SerializationHelper::GetStringFromBlob(bytes);
            auto jsonObject = JsonObject::Parse(str);
            auto status = (ResponseStatus)(uint32_t)jsonObject->Lookup("Status")->GetNumber();
            auto startupBackgroundApps = ref new Vector<String^>();
            auto jsonArray = jsonObject->Lookup("StartupBackgroundApps")->GetArray();
            for each (auto app in jsonArray)
            {
                startupBackgroundApps->Append(app->GetString());
            }
            return ref new ListStartupBackgroundAppsResponse(status, startupBackgroundApps);
        }

        virtual property DMMessageKind Tag {
            DMMessageKind get();
        }

        virtual property ResponseStatus Status {
            ResponseStatus get() { return status; }
        }

        property IVector<String^>^ StartupBackgroundApps {
            IVector<String^>^ get() { return startupBackgroundApps; }
        }
    };

}}}}
