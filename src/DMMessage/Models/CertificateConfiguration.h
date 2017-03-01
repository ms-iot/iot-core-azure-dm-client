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
    public ref class CertificateConfiguration sealed
    {
    public:
        property String^ rootCATrustedCertificates_Root;
        property String^ rootCATrustedCertificates_CA;
        property String^ rootCATrustedCertificates_TrustedPublisher;
        property String^ rootCATrustedCertificates_TrustedPeople;

        property String^ certificateStore_CA_System;
        property String^ certificateStore_Root_System;
        property String^ certificateStore_My_User;
        property String^ certificateStore_My_System;

        Blob^ Serialize(uint32_t tag) {

            JsonObject^ jsonObject = ref new JsonObject();
            jsonObject->Insert("rootCATrustedCertificates_Root", JsonValue::CreateStringValue(rootCATrustedCertificates_Root));
            jsonObject->Insert("rootCATrustedCertificates_CA", JsonValue::CreateStringValue(rootCATrustedCertificates_CA));
            jsonObject->Insert("rootCATrustedCertificates_TrustedPublisher", JsonValue::CreateStringValue(rootCATrustedCertificates_TrustedPublisher));
            jsonObject->Insert("rootCATrustedCertificates_TrustedPeople", JsonValue::CreateStringValue(rootCATrustedCertificates_TrustedPeople));

            jsonObject->Insert("certificateStore_CA_System", JsonValue::CreateStringValue(certificateStore_CA_System));
            jsonObject->Insert("certificateStore_Root_System", JsonValue::CreateStringValue(certificateStore_Root_System));
            jsonObject->Insert("certificateStore_My_User", JsonValue::CreateStringValue(certificateStore_My_User));
            jsonObject->Insert("certificateStore_My_System", JsonValue::CreateStringValue(certificateStore_My_System));

            return SerializationHelper::CreateBlobFromJson(tag, jsonObject);
        }

        static CertificateConfiguration^ Deserialize(Blob^ blob) {

            String^ str = SerializationHelper::GetStringFromBlob(blob);

            JsonObject^ jsonObject = JsonObject::Parse(str);
            auto certificateConfiguration = ref new CertificateConfiguration();
            certificateConfiguration->rootCATrustedCertificates_Root = jsonObject->Lookup("rootCATrustedCertificates_Root")->GetString();
            certificateConfiguration->rootCATrustedCertificates_CA = jsonObject->Lookup("rootCATrustedCertificates_CA")->GetString();
            certificateConfiguration->rootCATrustedCertificates_TrustedPublisher = jsonObject->Lookup("rootCATrustedCertificates_TrustedPublisher")->GetString();
            certificateConfiguration->rootCATrustedCertificates_TrustedPeople = jsonObject->Lookup("rootCATrustedCertificates_TrustedPeople")->GetString();

            certificateConfiguration->certificateStore_CA_System = jsonObject->Lookup("certificateStore_CA_System")->GetString();
            certificateConfiguration->certificateStore_Root_System = jsonObject->Lookup("certificateStore_Root_System")->GetString();
            certificateConfiguration->certificateStore_My_User = jsonObject->Lookup("certificateStore_My_User")->GetString();
            certificateConfiguration->certificateStore_My_System = jsonObject->Lookup("certificateStore_My_System")->GetString();

            return certificateConfiguration;
        }
    };

    public ref class SetCertificateConfigurationRequest sealed : public IRequest
    {
    public:
        SetCertificateConfigurationRequest(CertificateConfiguration^ certificateConfiguration)
        {
            Configuration = certificateConfiguration;
        }

        property CertificateConfiguration^ Configuration;

        virtual Blob^ Serialize() {
            return Configuration->Serialize((uint32_t)Tag);
        }

        static IDataPayload^ Deserialize(Blob^ blob) {

            CertificateConfiguration^ certificateConfiguration = CertificateConfiguration::Deserialize(blob);
             return ref new SetCertificateConfigurationRequest(certificateConfiguration);
        }

        virtual property DMMessageKind Tag {
            DMMessageKind get();
        }
    };

    public ref class GetCertificateConfigurationRequest sealed : public IRequest
    {
    public:
        GetCertificateConfigurationRequest() {}

        virtual Blob^ Serialize() {
            return SerializationHelper::CreateEmptyBlob((uint32_t)Tag);
        }

        static IDataPayload^ Deserialize(Blob^ blob) {
            assert(blob->Tag == DMMessageKind::GetCertificateConfiguration);
            return ref new GetCertificateConfigurationRequest();
        }

        virtual property DMMessageKind Tag {
            DMMessageKind get();
        }
    };

    public ref class GetCertificateConfigurationResponse sealed : public IResponse
    {
        StatusCodeResponse statusCodeResponse;
    public:
        property CertificateConfiguration^ Configuration;

        GetCertificateConfigurationResponse(ResponseStatus status, CertificateConfiguration^ certificateConfiguration) : statusCodeResponse(status, this->Tag)
        {
            Configuration = certificateConfiguration;
        }

        virtual Blob^ Serialize() {
            return Configuration->Serialize((uint32_t)Tag);
        }

        static IDataPayload^ Deserialize(Blob^ blob) {
            CertificateConfiguration^ certificateConfiguration = CertificateConfiguration::Deserialize(blob);
            return ref new GetCertificateConfigurationResponse(ResponseStatus::Success, certificateConfiguration);
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