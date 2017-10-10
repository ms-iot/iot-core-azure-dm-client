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
    public ref class SetTimeInfoRequestData sealed
    {
    public:
        property String^ ntpServer;

        property bool    dynamicDaylightTimeDisabled;

        property String^ timeZoneKeyName;

        property int     timeZoneBias;

        property String^ timeZoneStandardName;
        property String^ timeZoneStandardDate;
        property int     timeZoneStandardBias;
        property int     timeZoneStandardDayOfWeek;

        property String^ timeZoneDaylightName;
        property String^ timeZoneDaylightDate;
        property int     timeZoneDaylightBias;
        property int     timeZoneDaylightDayOfWeek;

        Blob^ Serialize(uint32_t tag)
        {
            JsonObject^ jsonObject = ref new JsonObject();

            jsonObject->Insert("ntpServer", JsonValue::CreateStringValue(ntpServer));

            jsonObject->Insert("dynamicDaylightTimeDisabled", JsonValue::CreateBooleanValue(dynamicDaylightTimeDisabled));

            jsonObject->Insert("timeZoneKeyName", JsonValue::CreateStringValue(timeZoneKeyName));

            jsonObject->Insert("timeZoneBias", JsonValue::CreateNumberValue(timeZoneBias));

            jsonObject->Insert("timeZoneStandardName", JsonValue::CreateStringValue(timeZoneStandardName));
            jsonObject->Insert("timeZoneStandardDate", JsonValue::CreateStringValue(timeZoneStandardDate));
            jsonObject->Insert("timeZoneStandardBias", JsonValue::CreateNumberValue(timeZoneStandardBias));
            jsonObject->Insert("timeZoneStandardDayOfWeek", JsonValue::CreateNumberValue(timeZoneStandardDayOfWeek));

            jsonObject->Insert("timeZoneDaylightName", JsonValue::CreateStringValue(timeZoneDaylightName));
            jsonObject->Insert("timeZoneDaylightDate", JsonValue::CreateStringValue(timeZoneDaylightDate));
            jsonObject->Insert("timeZoneDaylightBias", JsonValue::CreateNumberValue(timeZoneDaylightBias));
            jsonObject->Insert("timeZoneDaylightDayOfWeek", JsonValue::CreateNumberValue(timeZoneDaylightDayOfWeek));

            return SerializationHelper::CreateBlobFromJson((uint32_t)tag, jsonObject);
        }

        static SetTimeInfoRequestData^ Deserialize(Blob^ blob)
        {
            String^ str = SerializationHelper::GetStringFromBlob(blob);
            JsonObject^ jsonObject = JsonObject::Parse(str);
            auto result = ref new SetTimeInfoRequestData();

            result->ntpServer = jsonObject->Lookup("ntpServer")->GetString();

            result->dynamicDaylightTimeDisabled = jsonObject->Lookup("dynamicDaylightTimeDisabled")->GetBoolean();

            result->timeZoneKeyName = jsonObject->Lookup("timeZoneKeyName")->GetString();

            result->timeZoneBias = (int)jsonObject->Lookup("timeZoneBias")->GetNumber();

            result->timeZoneStandardName = jsonObject->Lookup("timeZoneStandardName")->GetString();
            result->timeZoneStandardDate = jsonObject->Lookup("timeZoneStandardDate")->GetString();
            result->timeZoneStandardBias = (int)jsonObject->Lookup("timeZoneStandardBias")->GetNumber();
            result->timeZoneStandardDayOfWeek = (int)jsonObject->Lookup("timeZoneStandardDayOfWeek")->GetNumber();

            result->timeZoneDaylightName = jsonObject->Lookup("timeZoneDaylightName")->GetString();
            result->timeZoneDaylightDate = jsonObject->Lookup("timeZoneDaylightDate")->GetString();
            result->timeZoneDaylightBias = (int)jsonObject->Lookup("timeZoneDaylightBias")->GetNumber();
            result->timeZoneDaylightDayOfWeek = (int)jsonObject->Lookup("timeZoneDaylightDayOfWeek")->GetNumber();

            return result;
        }
    };

    public ref class SetTimeInfoRequest sealed : public IRequest
    {
    public:
        property SetTimeInfoRequestData^ data;

        SetTimeInfoRequest(SetTimeInfoRequestData^ d)
        {
            data = d;
        }

        virtual Blob^ Serialize() {
            return data->Serialize((uint32_t)Tag);
        }

        static IDataPayload^ Deserialize(Blob^ blob) {
            SetTimeInfoRequestData^ d = SetTimeInfoRequestData::Deserialize(blob);
            return ref new SetTimeInfoRequest(d);
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

    public ref class GetTimeInfoResponseData sealed
    {
    public:
        property String^ localTime;
        property String^ ntpServer;

        property bool    dynamicDaylightTimeDisabled;

        property String^ timeZoneKeyName;

        property int     timeZoneBias;

        property String^ timeZoneStandardName;
        property String^ timeZoneStandardDate;
        property int     timeZoneStandardBias;
        property int     timeZoneStandardDayOfWeek;

        property String^ timeZoneDaylightName;
        property String^ timeZoneDaylightDate;
        property int     timeZoneDaylightBias;
        property int     timeZoneDaylightDayOfWeek;

        Blob^ Serialize(uint32_t tag)
        {
            JsonObject^ jsonObject = ref new JsonObject();
            jsonObject->Insert("localTime", JsonValue::CreateStringValue(localTime));
            jsonObject->Insert("ntpServer", JsonValue::CreateStringValue(ntpServer));

            jsonObject->Insert("dynamicDaylightTimeDisabled", JsonValue::CreateBooleanValue(dynamicDaylightTimeDisabled));

            jsonObject->Insert("timeZoneKeyName", JsonValue::CreateStringValue(timeZoneKeyName));

            jsonObject->Insert("timeZoneBias", JsonValue::CreateNumberValue(timeZoneBias));

            jsonObject->Insert("timeZoneStandardName", JsonValue::CreateStringValue(timeZoneStandardName));
            jsonObject->Insert("timeZoneStandardDate", JsonValue::CreateStringValue(timeZoneStandardDate));
            jsonObject->Insert("timeZoneStandardBias", JsonValue::CreateNumberValue(timeZoneStandardBias));
            jsonObject->Insert("timeZoneStandardDayOfWeek", JsonValue::CreateNumberValue(timeZoneStandardDayOfWeek));

            jsonObject->Insert("timeZoneDaylightName", JsonValue::CreateStringValue(timeZoneDaylightName));
            jsonObject->Insert("timeZoneDaylightDate", JsonValue::CreateStringValue(timeZoneDaylightDate));
            jsonObject->Insert("timeZoneDaylightBias", JsonValue::CreateNumberValue(timeZoneDaylightBias));
            jsonObject->Insert("timeZoneDaylightDayOfWeek", JsonValue::CreateNumberValue(timeZoneDaylightDayOfWeek));
            return SerializationHelper::CreateBlobFromJson((uint32_t)tag, jsonObject);
        }

        static GetTimeInfoResponseData^ Deserialize(Blob^ blob)
        {
            String^ str = SerializationHelper::GetStringFromBlob(blob);
            JsonObject^ jsonObject = JsonObject::Parse(str);
            auto result = ref new GetTimeInfoResponseData();

            result->localTime = jsonObject->Lookup("localTime")->GetString();
            result->ntpServer = jsonObject->Lookup("ntpServer")->GetString();

            result->dynamicDaylightTimeDisabled = jsonObject->Lookup("dynamicDaylightTimeDisabled")->GetBoolean();

            result->timeZoneKeyName = jsonObject->Lookup("timeZoneKeyName")->GetString();

            result->timeZoneBias = (int)jsonObject->Lookup("timeZoneBias")->GetNumber();

            result->timeZoneStandardName = jsonObject->Lookup("timeZoneStandardName")->GetString();
            result->timeZoneStandardDate = jsonObject->Lookup("timeZoneStandardDate")->GetString();
            result->timeZoneStandardBias = (int)jsonObject->Lookup("timeZoneStandardBias")->GetNumber();
            result->timeZoneStandardDayOfWeek = (int)jsonObject->Lookup("timeZoneStandardDayOfWeek")->GetNumber();

            result->timeZoneDaylightName = jsonObject->Lookup("timeZoneDaylightName")->GetString();
            result->timeZoneDaylightDate = jsonObject->Lookup("timeZoneDaylightDate")->GetString();
            result->timeZoneDaylightBias = (int)jsonObject->Lookup("timeZoneDaylightBias")->GetNumber();
            result->timeZoneDaylightDayOfWeek = (int)jsonObject->Lookup("timeZoneDaylightDayOfWeek")->GetNumber();
            return result;
        }
    };

    public ref class GetTimeInfoResponse sealed : public IResponse
    {
        StatusCodeResponse statusCodeResponse;

    public:
        property GetTimeInfoResponseData^ data;

        GetTimeInfoResponse(ResponseStatus status, GetTimeInfoResponseData^ d) : statusCodeResponse(status, this->Tag)
        {
            data = d;
        }

        virtual Blob^ Serialize() {
            return data->Serialize((uint32_t)Tag);
        }

        static IDataPayload^ Deserialize(Blob^ blob) {
            GetTimeInfoResponseData^ d = GetTimeInfoResponseData::Deserialize(blob);
            return ref new GetTimeInfoResponse(ResponseStatus::Success, d);
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

