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
    public ref class DeviceHealthAttestationVerifyHealthRequest sealed : public IRequest
    {
    public:
        property String^ HealthAttestationServerEndpoint;

        virtual Blob^ Serialize() {
            JsonObject^ jsonObject = ref new JsonObject();
            jsonObject->Insert("HealthAttestationServerEndpoint", JsonValue::CreateStringValue(HealthAttestationServerEndpoint));
            return SerializationHelper::CreateBlobFromJson((uint32_t)Tag, jsonObject);
        }

        static IDataPayload^ Deserialize(Blob^ blob) {
            assert(blob->Tag == DMMessageKind::DeviceHealthAttestationVerifyHealth);

            String^ str = SerializationHelper::GetStringFromBlob(blob);

            JsonObject^ jsonObject = JsonObject::Parse(str);
            auto request = ref new DeviceHealthAttestationVerifyHealthRequest();
            request->HealthAttestationServerEndpoint = jsonObject->Lookup("HealthAttestationServerEndpoint")->GetString();

            return request;
        }

        virtual property DMMessageKind Tag {
            DMMessageKind get();
        }
    };


    public ref class DeviceHealthAttestationGetReportRequest sealed : public IRequest
    {
    public:
        property String^ Nonce;

        virtual Blob^ Serialize() {
            JsonObject^ jsonObject = ref new JsonObject();
            jsonObject->Insert("Nonce", JsonValue::CreateStringValue(Nonce));
            return SerializationHelper::CreateBlobFromJson((uint32_t)Tag, jsonObject);
        }

        static IDataPayload^ Deserialize(Blob^ blob) {
            assert(blob->Tag == DMMessageKind::DeviceHealthAttestationGetReport);

            String^ str = SerializationHelper::GetStringFromBlob(blob);

            JsonObject^ jsonObject = JsonObject::Parse(str);
            auto request = ref new DeviceHealthAttestationGetReportRequest();
            request->Nonce = jsonObject->Lookup("Nonce")->GetString();

            return request;
        }

        virtual property DMMessageKind Tag {
            DMMessageKind get();
        }
    };

    public ref class DeviceHealthAttestationGetReportResponse sealed : IResponse
    {
        String^ healthCertificate;
        String^ correlationId;

    public:
        DeviceHealthAttestationGetReportResponse(String^ healthCertificate, String^ correlationId) :
            healthCertificate(healthCertificate), correlationId(correlationId) {}

        virtual Blob^ Serialize() {
            JsonObject^ jsonObject = ref new JsonObject();
            jsonObject->Insert("HealthCertificate", JsonValue::CreateStringValue(healthCertificate));
            jsonObject->Insert("CorrelationId", JsonValue::CreateStringValue(correlationId));
            return SerializationHelper::CreateBlobFromJson((uint32_t)Tag, jsonObject);
        }

        static IDataPayload^ Deserialize(Blob^ bytes) {
            String^ str = SerializationHelper::GetStringFromBlob(bytes);
            JsonObject^ jsonObject = JsonObject::Parse(str);
            String^ healthCertificate = jsonObject->Lookup("HealthCertificate")->GetString();
            String^ correlationId = jsonObject->Lookup("CorrelationId")->GetString();
            return ref new DeviceHealthAttestationGetReportResponse(healthCertificate, correlationId);
        }

        virtual property DMMessageKind Tag {
            DMMessageKind get();
        }

        virtual property ResponseStatus Status {
            ResponseStatus get() { return ResponseStatus::Success; }
        }

        property String^ HealthCertificate {
            String^ get() { return healthCertificate; }
        }

        property String^ CorrelationId {
            String^ get() { return correlationId; }
        }
    };
}
}}}
