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
    public ref class GetDeviceInfoRequest sealed : public IRequest
    {
    public:
        virtual Blob^ Serialize() {
            return SerializationHelper::CreateEmptyBlob((uint32_t)Tag);
        }

        static IDataPayload^ Deserialize(Blob^ bytes) {
            auto result = ref new GetDeviceInfoRequest();
            return result;
        }

        virtual property DMMessageKind Tag {
            DMMessageKind get();
        }
    };

    public ref class GetDeviceInfoResponse sealed : public IResponse
    {
        StatusCodeResponse statusCodeResponse;
    public:
        property String^ id;
        property String^ manufacturer;
        property String^ model;
        property String^ dmVer;
        property String^ lang;

        property String^ type;
        property String^ oem;
        property String^ hwVer;
        property String^ fwVer;
        property String^ osVer;

        property String^ platform;
        property String^ processorType;
        property String^ radioSwVer;
        property String^ displayResolution;
        property String^ commercializationOperator;

        property String^ processorArchitecture;
        property String^ name;
        property String^ totalStorage;
        property String^ totalMemory;
        property String^ secureBootState;

        property String^ osEdition;
        property String^ batteryStatus;
        property String^ batteryRemaining;
        property String^ batteryRuntime;

        GetDeviceInfoResponse(ResponseStatus status) : statusCodeResponse(status, this->Tag) {}

        virtual Blob^ Serialize() {
            JsonObject^ jsonObject = ref new JsonObject();

            jsonObject->Insert("id", JsonValue::CreateStringValue(id));
            jsonObject->Insert("manufacturer", JsonValue::CreateStringValue(manufacturer));
            jsonObject->Insert("model", JsonValue::CreateStringValue(model));
            jsonObject->Insert("dmVer", JsonValue::CreateStringValue(dmVer));
            jsonObject->Insert("lang", JsonValue::CreateStringValue(lang));

            jsonObject->Insert("type", JsonValue::CreateStringValue(type));
            jsonObject->Insert("oem", JsonValue::CreateStringValue(oem));
            jsonObject->Insert("hwVer", JsonValue::CreateStringValue(hwVer));
            jsonObject->Insert("fwVer", JsonValue::CreateStringValue(fwVer));
            jsonObject->Insert("osVer", JsonValue::CreateStringValue(osVer));

            jsonObject->Insert("platform", JsonValue::CreateStringValue(platform));
            jsonObject->Insert("processorType", JsonValue::CreateStringValue(processorType));
            jsonObject->Insert("radioSwVer", JsonValue::CreateStringValue(radioSwVer));
            jsonObject->Insert("displayResolution", JsonValue::CreateStringValue(displayResolution));
            jsonObject->Insert("commercializationOperator", JsonValue::CreateStringValue(commercializationOperator));

            jsonObject->Insert("processorArchitecture", JsonValue::CreateStringValue(processorArchitecture));
            jsonObject->Insert("name", JsonValue::CreateStringValue(name));
            jsonObject->Insert("totalStorage", JsonValue::CreateStringValue(totalStorage));
            jsonObject->Insert("totalMemory", JsonValue::CreateStringValue(totalMemory));
            jsonObject->Insert("secureBootState", JsonValue::CreateStringValue(secureBootState));

            jsonObject->Insert("osEdition", JsonValue::CreateStringValue(osEdition));
            jsonObject->Insert("batteryStatus", JsonValue::CreateStringValue(batteryStatus));
            jsonObject->Insert("batteryRemaining", JsonValue::CreateStringValue(batteryRemaining));
            jsonObject->Insert("batteryRuntime", JsonValue::CreateStringValue(batteryRuntime));

            return SerializationHelper::CreateBlobFromJson((uint32_t)Tag, jsonObject);
        }

        static IDataPayload^ Deserialize(Blob^ blob) {
            String^ str = SerializationHelper::GetStringFromBlob(blob);
            JsonObject^ jsonObject = JsonObject::Parse(str);
            auto result = ref new GetDeviceInfoResponse(ResponseStatus::Success);

            result->id = jsonObject->Lookup("id")->GetString();
            result->manufacturer = jsonObject->Lookup("manufacturer")->GetString();
            result->model = jsonObject->Lookup("model")->GetString();
            result->dmVer = jsonObject->Lookup("dmVer")->GetString();
            result->lang = jsonObject->Lookup("lang")->GetString();

            result->type = jsonObject->Lookup("type")->GetString();
            result->oem = jsonObject->Lookup("oem")->GetString();
            result->hwVer = jsonObject->Lookup("hwVer")->GetString();
            result->fwVer = jsonObject->Lookup("fwVer")->GetString();
            result->osVer = jsonObject->Lookup("osVer")->GetString();

            result->platform = jsonObject->Lookup("platform")->GetString();
            result->processorType = jsonObject->Lookup("processorType")->GetString();
            result->radioSwVer = jsonObject->Lookup("radioSwVer")->GetString();
            result->displayResolution = jsonObject->Lookup("displayResolution")->GetString();
            result->commercializationOperator = jsonObject->Lookup("commercializationOperator")->GetString();

            result->processorArchitecture = jsonObject->Lookup("processorArchitecture")->GetString();
            result->name = jsonObject->Lookup("name")->GetString();
            result->totalStorage = jsonObject->Lookup("totalStorage")->GetString();
            result->totalMemory = jsonObject->Lookup("totalMemory")->GetString();
            result->secureBootState = jsonObject->Lookup("secureBootState")->GetString();

            result->osEdition = jsonObject->Lookup("osEdition")->GetString();
            result->batteryStatus = jsonObject->Lookup("batteryStatus")->GetString();
            result->batteryRemaining = jsonObject->Lookup("batteryRemaining")->GetString();
            result->batteryRuntime = jsonObject->Lookup("batteryRuntime")->GetString();

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

