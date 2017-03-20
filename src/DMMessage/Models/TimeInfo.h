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
    public ref class SetTimeInfoRequest sealed : public IRequest
    {
    public:
        property String^ ntpServer;
        property int     timeZoneBias;
        property String^ timeZoneStandardName;
        property String^ timeZoneStandardDate;
        property int     timeZoneStandardBias;
        property String^ timeZoneDaylightName;
        property String^ timeZoneDaylightDate;
        property int     timeZoneDaylightBias;

        virtual Blob^ Serialize() {
            JsonObject^ jsonObject = ref new JsonObject();
            jsonObject->Insert("ntpServer", JsonValue::CreateStringValue(ntpServer));
            jsonObject->Insert("timeZoneBias", JsonValue::CreateNumberValue(timeZoneBias));
            jsonObject->Insert("timeZoneStandardName", JsonValue::CreateStringValue(timeZoneStandardName));
            jsonObject->Insert("timeZoneStandardDate", JsonValue::CreateStringValue(timeZoneStandardDate));
            jsonObject->Insert("timeZoneStandardBias", JsonValue::CreateNumberValue(timeZoneStandardBias));
            jsonObject->Insert("timeZoneDaylightName", JsonValue::CreateStringValue(timeZoneDaylightName));
            jsonObject->Insert("timeZoneDaylightDate", JsonValue::CreateStringValue(timeZoneDaylightDate));
            jsonObject->Insert("timeZoneDaylightBias", JsonValue::CreateNumberValue(timeZoneDaylightBias));
            return SerializationHelper::CreateBlobFromJson((uint32_t)Tag, jsonObject);
        }

        static IDataPayload^ Deserialize(Blob^ blob) {
            String^ str = SerializationHelper::GetStringFromBlob(blob);

            JsonObject^ jsonObject = JsonObject::Parse(str);
            auto setTimeInfo = ref new SetTimeInfoRequest();
            setTimeInfo->ntpServer = jsonObject->Lookup("ntpServer")->GetString();
            setTimeInfo->timeZoneBias = (int)jsonObject->Lookup("timeZoneBias")->GetNumber();
            setTimeInfo->timeZoneStandardName = jsonObject->Lookup("timeZoneStandardName")->GetString();
            setTimeInfo->timeZoneStandardDate = jsonObject->Lookup("timeZoneStandardDate")->GetString();
            setTimeInfo->timeZoneStandardBias = (int)jsonObject->Lookup("timeZoneStandardBias")->GetNumber();
            setTimeInfo->timeZoneDaylightName = jsonObject->Lookup("timeZoneDaylightName")->GetString();
            setTimeInfo->timeZoneDaylightDate = jsonObject->Lookup("timeZoneDaylightDate")->GetString();
            setTimeInfo->timeZoneDaylightBias = (int)jsonObject->Lookup("timeZoneDaylightBias")->GetNumber();

            return setTimeInfo;
        }

        virtual property DMMessageKind Tag {
            DMMessageKind get();
        }
    };

    public ref class GetTimeInfoRequest sealed : public IRequest
    {
    public:
        virtual Blob^ Serialize() {
            return SerializationHelper::CreateEmptyBlob((uint32_t)Tag);
        }

        static IDataPayload^ Deserialize(Blob^ bytes) {
            auto result = ref new GetTimeInfoRequest();
            return result;
        }

        virtual property DMMessageKind Tag {
            DMMessageKind get();
        }
    };

    public ref class GetTimeInfoResponse sealed : public IResponse
    {
        StatusCodeResponse statusCodeResponse;
    public:
        property String^ localTime;
        property String^ ntpServer;
        property int     timeZoneBias;
        property String^ timeZoneStandardName;
        property String^ timeZoneStandardDate;
        property int     timeZoneStandardBias;
        property String^ timeZoneDaylightName;
        property String^ timeZoneDaylightDate;
        property int     timeZoneDaylightBias;

        GetTimeInfoResponse(ResponseStatus status) : statusCodeResponse(status, this->Tag) {}

        virtual Blob^ Serialize() {
            JsonObject^ jsonObject = ref new JsonObject();
            jsonObject->Insert("localTime", JsonValue::CreateStringValue(localTime));
            jsonObject->Insert("ntpServer", JsonValue::CreateStringValue(ntpServer));
            jsonObject->Insert("timeZoneBias", JsonValue::CreateNumberValue(timeZoneBias));
            jsonObject->Insert("timeZoneStandardName", JsonValue::CreateStringValue(timeZoneStandardName));
            jsonObject->Insert("timeZoneStandardDate", JsonValue::CreateStringValue(timeZoneStandardDate));
            jsonObject->Insert("timeZoneStandardBias", JsonValue::CreateNumberValue(timeZoneStandardBias));
            jsonObject->Insert("timeZoneDaylightName", JsonValue::CreateStringValue(timeZoneDaylightName));
            jsonObject->Insert("timeZoneDaylightDate", JsonValue::CreateStringValue(timeZoneDaylightDate));
            jsonObject->Insert("timeZoneDaylightBias", JsonValue::CreateNumberValue(timeZoneDaylightBias));
            return SerializationHelper::CreateBlobFromJson((uint32_t)Tag, jsonObject);
        }

        static IDataPayload^ Deserialize(Blob^ blob) {
            String^ str = SerializationHelper::GetStringFromBlob(blob);
            JsonObject^ jsonObject = JsonObject::Parse(str);
            auto result = ref new GetTimeInfoResponse(ResponseStatus::Success);
            result->localTime = jsonObject->Lookup("localTime")->GetString();
            result->ntpServer = jsonObject->Lookup("ntpServer")->GetString();
            result->timeZoneBias = (int)jsonObject->Lookup("timeZoneBias")->GetNumber();
            result->timeZoneStandardName = jsonObject->Lookup("timeZoneStandardName")->GetString();
            result->timeZoneStandardDate = jsonObject->Lookup("timeZoneStandardDate")->GetString();
            result->timeZoneStandardBias = (int)jsonObject->Lookup("timeZoneStandardBias")->GetNumber();
            result->timeZoneDaylightName = jsonObject->Lookup("timeZoneDaylightName")->GetString();
            result->timeZoneDaylightDate = jsonObject->Lookup("timeZoneDaylightDate")->GetString();
            result->timeZoneDaylightBias = (int)jsonObject->Lookup("timeZoneDaylightBias")->GetNumber();
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

