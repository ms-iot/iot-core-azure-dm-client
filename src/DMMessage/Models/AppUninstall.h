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
    public ref class AppUninstallRequestData sealed
    {
    public:
        AppUninstallRequestData()
        {
            PackageFamilyName = ref new Platform::String();
            StoreApp = true;
        }
        AppUninstallRequestData(String^ packageFamilyName, bool storeApp)
        {
            PackageFamilyName = packageFamilyName;
            StoreApp = storeApp;
        }
        property String^ PackageFamilyName;
        property bool StoreApp;
    };

    public ref class AppUninstallRequest sealed : public IRequest
    {
    public:
        property AppUninstallRequestData^ data;

        AppUninstallRequest(AppUninstallRequestData^ d) 
        {
            data = d;
        }

        virtual Blob^ Serialize() {
            JsonObject^ jsonObject = ref new JsonObject();
            jsonObject->Insert("PackageFamilyName", JsonValue::CreateStringValue(data->PackageFamilyName));
            jsonObject->Insert("StoreApp", JsonValue::CreateBooleanValue(data->StoreApp));
            return SerializationHelper::CreateBlobFromJson((uint32_t)Tag, jsonObject);
        }

        static IDataPayload^ Deserialize(Blob^ bytes) {
            String^ str = SerializationHelper::GetStringFromBlob(bytes);
            JsonObject^ jsonObject = JsonObject::Parse(str);
            auto appId = jsonObject->Lookup("PackageFamilyName")->GetString();
            auto start = jsonObject->Lookup("StoreApp")->GetBoolean();
            auto d = ref new Microsoft::Devices::Management::Message::AppUninstallRequestData(appId, start);
            return ref new AppUninstallRequest(d);
        }

        virtual property DMMessageKind Tag {
            DMMessageKind get();
        }
    };

    public ref class AppUninstallResponseData sealed
    {
    public:
        property int errorCode;
        property String^ errorMessage;

        Blob^ Serialize(uint32_t tag) {
            JsonObject^ jsonObject = ref new JsonObject();

            jsonObject->Insert("errorCode", JsonValue::CreateNumberValue(errorCode));
            jsonObject->Insert("errorMessage", JsonValue::CreateStringValue(errorMessage));

            return SerializationHelper::CreateBlobFromJson(tag, jsonObject);
        }

        static AppUninstallResponseData^ Deserialize(Blob^ blob) {
            String^ str = SerializationHelper::GetStringFromBlob(blob);
            JsonObject^ jsonObject = JsonObject::Parse(str);
            auto result = ref new AppUninstallResponseData();

            result->errorCode = static_cast<int>(jsonObject->Lookup("errorCode")->GetNumber());
            result->errorMessage = jsonObject->Lookup("errorMessage")->GetString();

            return result;
        }
    };

    public ref class AppUninstallResponse sealed : public IResponse
    {
        StatusCodeResponse statusCodeResponse;
    public:
        property AppUninstallResponseData^ data;

        AppUninstallResponse(ResponseStatus status, AppUninstallResponseData^ d) :
            statusCodeResponse(status, this->Tag)
        {
            data = d;
        }

        virtual Blob^ Serialize() {
            return data->Serialize((uint32_t)Tag);
        }

        static IDataPayload^ Deserialize(Blob^ blob) {
            AppUninstallResponseData^ d = AppUninstallResponseData::Deserialize(blob);
            return ref new AppUninstallResponse(ResponseStatus::Success, d);
        }

        virtual property ResponseStatus Status {
            ResponseStatus get() { return statusCodeResponse.Status; }
        }

        virtual property DMMessageKind Tag {
            DMMessageKind get();
        }
    };
}}}}
