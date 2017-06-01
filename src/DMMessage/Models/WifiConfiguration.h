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
    public ref class WifiProfileConfiguration sealed
    {
    public:
        property String^ Name;
        property String^ Xml;
        property bool disableInternetConnectivityChecks;

        Blob^ Serialize(uint32_t tag) {

            JsonObject^ jsonObject = ref new JsonObject();
            jsonObject->Insert("Name", JsonValue::CreateStringValue(Name));
            jsonObject->Insert("Xml", JsonValue::CreateStringValue(Xml));
            jsonObject->Insert("disableInternetConnectivityChecks", JsonValue::CreateBooleanValue(disableInternetConnectivityChecks));

            return SerializationHelper::CreateBlobFromJson(tag, jsonObject);
        }

        static WifiProfileConfiguration^ Deserialize(Blob^ blob) {

            String^ str = SerializationHelper::GetStringFromBlob(blob);

            JsonObject^ jsonObject = JsonObject::Parse(str);
            auto wifiConfiguration = ref new WifiProfileConfiguration();
			wifiConfiguration->Name = jsonObject->GetNamedString("Name");
			wifiConfiguration->Xml = jsonObject->GetNamedString("Xml");
			wifiConfiguration->disableInternetConnectivityChecks = jsonObject->GetNamedBoolean("disableInternetConnectivityChecks");

            return wifiConfiguration;
        }
    };

    public ref class WifiConfiguration sealed
    {
    public:
        property IVector<WifiProfileConfiguration^>^ Profiles;
        property String^ Active;

        WifiConfiguration()
        {
            Active = ref new Platform::String();
            Profiles = ref new Vector<WifiProfileConfiguration^>();
        }
        WifiConfiguration(String^ active, IVector<WifiProfileConfiguration^>^ profiles)
        {
            Active = active;
            Profiles = profiles;
        }

        Blob^ Serialize(uint32_t tag) {

            JsonObject^ jsonObject = ref new JsonObject();
            jsonObject->Insert("active", JsonValue::CreateStringValue(Active));
            JsonObject^ jsonProfiles = ref new JsonObject();
            for each (auto profile in Profiles)
            {
                auto propMap = ref new JsonObject();
                propMap->Insert(ref new Platform::String(L"profile"), JsonValue::CreateStringValue(profile->Xml));
                propMap->Insert(ref new Platform::String(L"disableInternetConnectivityChecks"), JsonValue::CreateBooleanValue(profile->disableInternetConnectivityChecks));

                jsonProfiles->Insert(profile->Name, propMap);
            }
            jsonObject->Insert("profiles", jsonProfiles);

            return SerializationHelper::CreateBlobFromJson(tag, jsonObject);
        }

        static WifiConfiguration^ Deserialize(Blob^ blob) {

            String^ str = SerializationHelper::GetStringFromBlob(blob);

            JsonObject^ jsonObject = JsonObject::Parse(str);
            auto wifiConfiguration = ref new WifiConfiguration();
            wifiConfiguration->Active = jsonObject->GetNamedString("active");

            JsonObject^ profiles = jsonObject->GetNamedObject(ref new Platform::String(L"profiles"));
            for each (auto profile in profiles)
            {
                auto profileName = profile->Key;
                auto profileValue = profiles->GetNamedObject(profileName);

                auto wifiProfile = ref new WifiProfileConfiguration();
                wifiProfile->Name = profileName;
                wifiProfile->disableInternetConnectivityChecks = profileValue->GetNamedBoolean("disableInternetConnectivityChecks");
                wifiProfile->Xml = profileValue->GetNamedString("profile");

                wifiConfiguration->Profiles->Append(wifiProfile);
            }

            return wifiConfiguration;
        }
    };

	public ref class SetWifiConfigurationRequest sealed : public IRequest
    {
    public:
        property WifiConfiguration^ configuration;


		SetWifiConfigurationRequest(WifiConfiguration^ wifiConfiguration)
        {
            configuration = wifiConfiguration;
        }

        virtual Blob^ Serialize()
        {
            return configuration->Serialize((uint32_t)Tag);
        }

        static IDataPayload^ Deserialize(Blob^ blob)
        {
			WifiConfiguration^ wifiConfiguration = WifiConfiguration::Deserialize(blob);
             return ref new SetWifiConfigurationRequest(wifiConfiguration);
        }

        virtual property DMMessageKind Tag {
            DMMessageKind get();
        }
    };

    public ref class GetWifiConfigurationRequest sealed : public IRequest
    {
    public:
		GetWifiConfigurationRequest() {}

        virtual Blob^ Serialize() {
            return SerializationHelper::CreateEmptyBlob((uint32_t)Tag);
        }

        static IDataPayload^ Deserialize(Blob^ blob) {
            assert(blob->Tag == DMMessageKind::GetWifiConfiguration);
            return ref new GetWifiConfigurationRequest();
        }

        virtual property DMMessageKind Tag {
            DMMessageKind get();
        }
    };

    public ref class GetWifiConfigurationResponse sealed : public IResponse
    {
        StatusCodeResponse statusCodeResponse;
    public:
        property WifiConfiguration^ configuration;

		GetWifiConfigurationResponse(ResponseStatus status, WifiConfiguration^ wifiConfiguration) : statusCodeResponse(status, this->Tag)
        {
            configuration = wifiConfiguration;
        }

        virtual Blob^ Serialize() {
            return configuration->Serialize((uint32_t)Tag);
        }

        static IDataPayload^ Deserialize(Blob^ blob) {
            WifiConfiguration^ wifiConfiguration = WifiConfiguration::Deserialize(blob);
            return ref new GetWifiConfigurationResponse(ResponseStatus::Success, wifiConfiguration);
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
