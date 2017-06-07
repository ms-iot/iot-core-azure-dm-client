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
#include "ModelHelper.h"

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
        property String^ Path;
        property bool DisableInternetConnectivityChecks;
        property bool Uninstall;

        Blob^ Serialize(uint32_t tag) {

            JsonObject^ jsonObject = ref new JsonObject();
            jsonObject->Insert("name", JsonValue::CreateStringValue(Name));
            jsonObject->Insert("xml", JsonValue::CreateStringValue(Xml));
            jsonObject->Insert("profile", JsonValue::CreateStringValue(Xml));
            jsonObject->Insert("disableInternetConnectivityChecks", JsonValue::CreateBooleanValue(DisableInternetConnectivityChecks));
            jsonObject->Insert("uninstall", JsonValue::CreateBooleanValue(Uninstall));

            return SerializationHelper::CreateBlobFromJson(tag, jsonObject);
        }

        static WifiProfileConfiguration^ Deserialize(Blob^ blob) {

            String^ str = SerializationHelper::GetStringFromBlob(blob);

            JsonObject^ jsonObject = JsonObject::Parse(str);
            auto wifiConfiguration = ref new WifiProfileConfiguration();
			wifiConfiguration->Name = jsonObject->GetNamedString("name");
            wifiConfiguration->Path = jsonObject->GetNamedString("profile");
            wifiConfiguration->Xml = jsonObject->GetNamedString("xml");
            wifiConfiguration->DisableInternetConnectivityChecks = jsonObject->GetNamedBoolean("disableInternetConnectivityChecks");
            wifiConfiguration->Uninstall = jsonObject->GetNamedBoolean("uninstall");

            return wifiConfiguration;
        }
    };

    public ref class WifiConfiguration sealed
    {
    public:
        property IVector<WifiProfileConfiguration^>^ Profiles;
        property bool Reporting;
        property bool Applying;

        WifiConfiguration()
        {
            Profiles = ref new Vector<WifiProfileConfiguration^>();
            Applying = true;
            Reporting = true;
        }
        WifiConfiguration(String^ active, IVector<WifiProfileConfiguration^>^ profiles, bool applying, bool reporting)
        {
            Profiles = profiles;
            Applying = applying;
            Reporting = reporting;
        }

        Blob^ Serialize(uint32_t tag) {
            return DesiredAndReportedConfigurationHelper<WifiConfiguration>::Serialize(
                this, tag, [](JsonObject^ applyPropertiesObject, WifiConfiguration^ configObject) {
                    for each (auto profile in configObject->Profiles)
                    {
                        if (profile->Uninstall)
                        {
                            applyPropertiesObject->Insert(profile->Name, JsonValue::CreateStringValue(L"uninstall"));
                        }
                        else
                        {
                            auto propMap = ref new JsonObject();
                            propMap->Insert(ref new Platform::String(L"profile"), JsonValue::CreateStringValue(profile->Path));
                            propMap->Insert(ref new Platform::String(L"xml"), JsonValue::CreateStringValue(profile->Xml));
                            propMap->Insert(ref new Platform::String(L"disableInternetConnectivityChecks"), JsonValue::CreateBooleanValue(profile->DisableInternetConnectivityChecks));
                            propMap->Insert(ref new Platform::String(L"uninstall"), JsonValue::CreateBooleanValue(profile->Uninstall));
                            applyPropertiesObject->Insert(profile->Name, propMap);
                        }
                    }
                });
        }

        static WifiConfiguration^ Parse(Platform::String^ str) {
            return DesiredAndReportedConfigurationHelper<WifiConfiguration>::Parse(
                str, [](JsonObject^ applyPropertiesObject, WifiConfiguration^ configObject) {
                    for each (auto profile in applyPropertiesObject)
                    {
                        auto profileName = profile->Key;
                        auto profileValue = applyPropertiesObject->Lookup(profileName);

                        if (profileValue->ValueType == JsonValueType::Object)
                        {
                            auto profileValueObject = applyPropertiesObject->GetNamedObject(profileName);

                            auto wifiProfile = ref new WifiProfileConfiguration();
                            wifiProfile->Name = profileName;

                            wifiProfile->DisableInternetConnectivityChecks = profileValueObject->GetNamedBoolean("disableInternetConnectivityChecks", false);
                            wifiProfile->Path = profileValueObject->GetNamedString("profile");
                            wifiProfile->Xml = profileValueObject->GetNamedString("xml", L"");
                            wifiProfile->Uninstall = profileValueObject->GetNamedBoolean("uninstall", false);

                            configObject->Profiles->Append(wifiProfile);
                        }
                        else if (profileValue->ValueType == JsonValueType::String)
                        {
                            auto valueString = profileValue->GetString();
                            if (valueString == "uninstall")
                            {
                                auto wifiProfile = ref new WifiProfileConfiguration();
                                wifiProfile->Name = profileName;
                                wifiProfile->Uninstall = true;

                            }
                        }
                    }
                });
        }

        static WifiConfiguration^ Deserialize(Blob^ blob) {
            String^ str = SerializationHelper::GetStringFromBlob(blob);
            return Parse(str);
        }
    };

	public ref class SetWifiConfigurationRequest sealed : public IRequest
    {
    public:
        property WifiConfiguration^ Configuration;


        SetWifiConfigurationRequest(WifiConfiguration^ wifiConfiguration)
        {
            Configuration = wifiConfiguration;
        }

        virtual Blob^ Serialize()
        {
            return Configuration->Serialize((uint32_t)Tag);
        }

        static IDataPayload^ Deserialize(Blob^ blob)
        {
            WifiConfiguration^ wifi = WifiConfiguration::Deserialize(blob);
             return ref new SetWifiConfigurationRequest(wifi);
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
        property WifiConfiguration^ Configuration;

		GetWifiConfigurationResponse(ResponseStatus status, WifiConfiguration^ wifiConfiguration) : statusCodeResponse(status, this->Tag)
        {
            Configuration = wifiConfiguration;
        }

        virtual Blob^ Serialize() {
            return Configuration->Serialize((uint32_t)Tag);
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
