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

    public ref class StartupAppResponse sealed : public IResponse
    {
        StatusCodeResponse statusCodeResponse;
    public:
		StartupAppResponse(ResponseStatus status) : statusCodeResponse(status, this->Tag) {}
        virtual Blob^ Serialize() {
            return statusCodeResponse.Serialize();
        }

        static IDataPayload^ Deserialize(Blob^ bytes) {
            return ref new StartupAppResponse(StatusCodeResponse::Deserialize(bytes)->Status);
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
