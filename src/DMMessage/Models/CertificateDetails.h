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
    public ref class GetCertificateDetailsRequest sealed : public IRequest
    {
    public:
        GetCertificateDetailsRequest() {}

        property String^ path;
        property String^ hash;

        virtual Blob^ Serialize() {

            JsonObject^ jsonObject = ref new JsonObject();
            jsonObject->Insert("path", JsonValue::CreateStringValue(path));
            jsonObject->Insert("hash", JsonValue::CreateStringValue(hash));
            return SerializationHelper::CreateBlobFromJson((uint32_t)Tag, jsonObject);
        }

        static IDataPayload^ Deserialize(Blob^ blob) {
            assert(blob->Tag == DMMessageKind::GetCertificateDetails);

            String^ str = SerializationHelper::GetStringFromBlob(blob);

            JsonObject^ jsonObject = JsonObject::Parse(str);
            GetCertificateDetailsRequest^ getCertificateDetailsRequest = ref new GetCertificateDetailsRequest();
            getCertificateDetailsRequest->path = jsonObject->Lookup("path")->GetString();
            getCertificateDetailsRequest->hash = jsonObject->Lookup("hash")->GetString();

            return getCertificateDetailsRequest;
        }

        virtual property DMMessageKind Tag {
            DMMessageKind get();
        }
    };

    public ref class GetCertificateDetailsResponse sealed : public IResponse
    {
        StatusCodeResponse statusCodeResponse;
    public:
        property String^ IssuedBy;
        property String^ IssuedTo;
        property String^ ValidFrom;
        property String^ ValidTo;

        GetCertificateDetailsResponse(ResponseStatus status) : statusCodeResponse(status, this->Tag) {}

        virtual Blob^ Serialize() {

            JsonObject^ jsonObject = ref new JsonObject();
            jsonObject->Insert("IssuedBy", JsonValue::CreateStringValue(IssuedBy));
            jsonObject->Insert("IssuedTo", JsonValue::CreateStringValue(IssuedTo));
            jsonObject->Insert("ValidFrom", JsonValue::CreateStringValue(ValidFrom));
            jsonObject->Insert("ValidTo", JsonValue::CreateStringValue(ValidTo));

            return SerializationHelper::CreateBlobFromJson((uint32_t)Tag, jsonObject);
        }

        static IDataPayload^ Deserialize(Blob^ blob) {

            String^ str = SerializationHelper::GetStringFromBlob(blob);

            JsonObject^ jsonObject = JsonObject::Parse(str);
            auto getCertificateDetailsResponse = ref new GetCertificateDetailsResponse(ResponseStatus::Success);
            getCertificateDetailsResponse->IssuedBy = jsonObject->Lookup("IssuedBy")->GetString();
            getCertificateDetailsResponse->IssuedTo = jsonObject->Lookup("IssuedTo")->GetString();
            getCertificateDetailsResponse->ValidFrom = jsonObject->Lookup("ValidFrom")->GetString();
            getCertificateDetailsResponse->ValidTo = jsonObject->Lookup("ValidTo")->GetString();

            return getCertificateDetailsResponse;
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