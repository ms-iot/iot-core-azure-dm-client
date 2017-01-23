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
    public ref class TimeInfoRequest sealed : public IRequest
    {
    public:
        virtual Blob^ Serialize() {
            return SerializationHelper::CreateEmptyBlob((uint32_t)Tag);
        }

        static IDataPayload^ Deserialize(Blob^ bytes) {
            auto result = ref new TimeInfoRequest();
            return result;
        }

        virtual property DMMessageKind Tag {
            DMMessageKind get() { return DMMessageKind::GetTimeInfo; }
        }
    };

    public ref class TimeInfoResponse sealed : public IResponse
    {
        StatusCodeResponse statusCodeResponse;
    public:
        property String^ LocalTime;
        property String^ NtpServer;
        property int     TimeZoneBias;
        property String^ TimeZoneStandardName;
        property String^ TimeZoneStandardDate;
        property int     TimeZoneStandardBias;
        property String^ TimeZoneDaylightName;
        property String^ TimeZoneDaylightDate;
        property int     TimeZoneDaylightBias;

        TimeInfoResponse(ResponseStatus status) : statusCodeResponse(status, this->Tag) {}

        virtual Blob^ Serialize() {
            JsonObject^ jsonObject = ref new JsonObject();
            jsonObject->Insert("LocalTime", JsonValue::CreateStringValue(LocalTime));
            jsonObject->Insert("NtpServer", JsonValue::CreateStringValue(NtpServer));
            jsonObject->Insert("TimeZoneBias", JsonValue::CreateNumberValue(TimeZoneBias));
            jsonObject->Insert("TimeZoneStandardName", JsonValue::CreateStringValue(TimeZoneStandardName));
            jsonObject->Insert("TimeZoneStandardDate", JsonValue::CreateStringValue(TimeZoneStandardDate));
            jsonObject->Insert("TimeZoneStandardBias", JsonValue::CreateNumberValue(TimeZoneStandardBias));
            jsonObject->Insert("TimeZoneDaylightName", JsonValue::CreateStringValue(TimeZoneDaylightName));
            jsonObject->Insert("TimeZoneDaylightDate", JsonValue::CreateStringValue(TimeZoneDaylightDate));
            jsonObject->Insert("TimeZoneDaylightBias", JsonValue::CreateNumberValue(TimeZoneDaylightBias));
            return SerializationHelper::CreateBlobFromJson((uint32_t)Tag, jsonObject);
        }

        static IDataPayload^ Deserialize(Blob^ blob) {
            String^ str = SerializationHelper::GetStringFromBlob(blob);
            JsonObject^ jsonObject = JsonObject::Parse(str);
            auto result = ref new TimeInfoResponse(ResponseStatus::Success);
            result->LocalTime = jsonObject->Lookup("LocalTime")->GetString();
            result->NtpServer = jsonObject->Lookup("NtpServer")->GetString();
            result->TimeZoneBias = (int)jsonObject->Lookup("TimeZoneBias")->GetNumber();
            result->TimeZoneStandardName = jsonObject->Lookup("TimeZoneStandardName")->GetString();
            result->TimeZoneStandardDate = jsonObject->Lookup("TimeZoneStandardDate")->GetString();
            result->TimeZoneStandardBias = (int)jsonObject->Lookup("TimeZoneStandardBias")->GetNumber();
            result->TimeZoneDaylightName = jsonObject->Lookup("TimeZoneDaylightName")->GetString();
            result->TimeZoneDaylightDate = jsonObject->Lookup("TimeZoneDaylightDate")->GetString();
            result->TimeZoneDaylightBias = (int)jsonObject->Lookup("TimeZoneDaylightBias")->GetNumber();
            return result;
        }

        virtual property ResponseStatus Status {
            ResponseStatus get() { return statusCodeResponse.Status; }
        }

        virtual property DMMessageKind Tag {
            DMMessageKind get() { return DMMessageKind::GetTimeInfo; }
        }
    };
}
}}}
#pragma once
