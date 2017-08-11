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

#include <collection.h>

using namespace Platform;
using namespace Platform::Metadata;
using namespace Platform::Collections;
using namespace Windows::Data::Json;
using namespace Windows::Foundation::Collections;

namespace Microsoft { namespace Devices { namespace Management { namespace Message
{
    public enum class StartUpType { None, Foreground, Background };

    public ref class AppInstallRequestData sealed
    {
    public:
        AppInstallRequestData()
        {
            PackageFamilyName = ref new String();
            AppxPath = ref new String();
            Dependencies = ref new Vector<String^>();
            CertFile = ref new String();
            CertStore = ref new String();
            IsDMSelfUpdate = false;
        }
        AppInstallRequestData(String^ packageFamilyName, StartUpType startUp, String^ appxPath, IVector<String^>^ dependencies, String^ certFile, String^ certStore, bool isDMSelfUpdate)
        {
            PackageFamilyName = packageFamilyName;
            StartUp = startUp;
            AppxPath = appxPath;
            Dependencies = dependencies;
            CertFile = certFile;
            CertStore = certStore;
            IsDMSelfUpdate = isDMSelfUpdate;
        }
        property String^ PackageFamilyName;
        property StartUpType StartUp;
        property String^ AppxPath;
        property IVector<String^>^ Dependencies;
        property String^ CertFile;
        property String^ CertStore;
        property bool IsDMSelfUpdate;
    };

    public ref class AppInstallRequest sealed : public IRequest
    {
    public:
        property AppInstallRequestData^ data;

        AppInstallRequest(AppInstallRequestData^ d)
        {
            data = d;
        }

        virtual Blob^ Serialize() {
            JsonObject^ jsonObject = ref new JsonObject();
            jsonObject->Insert("PackageFamilyName", JsonValue::CreateStringValue(data->PackageFamilyName));
            jsonObject->Insert("StartUp", JsonValue::CreateNumberValue(static_cast<int>(data->StartUp)));
            jsonObject->Insert("AppxPath", JsonValue::CreateStringValue(data->AppxPath));
            JsonArray^ jsonArray = ref new JsonArray();
            for each (auto dep in data->Dependencies)
            {
                jsonArray->Append(JsonValue::CreateStringValue(dep));
            }
            jsonObject->Insert("Dependencies", jsonArray);
            jsonObject->Insert("CertFile", JsonValue::CreateStringValue(data->CertFile));
            jsonObject->Insert("CertStore", JsonValue::CreateStringValue(data->CertStore));
            jsonObject->Insert("IsDMSelfUpdate", JsonValue::CreateBooleanValue(data->IsDMSelfUpdate));

            return SerializationHelper::CreateBlobFromJson((uint32_t)Tag, jsonObject);
        }

        static IDataPayload^ Deserialize(Blob^ bytes) {
            String^ str = SerializationHelper::GetStringFromBlob(bytes);
            JsonObject^ jsonObject = JsonObject::Parse(str);
            auto packageFamilyName = jsonObject->GetNamedString("PackageFamilyName");
            auto startUp = static_cast<StartUpType>(static_cast<int>(jsonObject->GetNamedNumber("StartUp")));
            auto appxPath = jsonObject->GetNamedString("AppxPath");
            auto dependencies = jsonObject->GetNamedArray("Dependencies");
            auto depsVector = ref new Vector<String^>();
            for each (auto dep in dependencies)
            {
                depsVector->Append(dep->GetString());
            }
            auto certFile = jsonObject->GetNamedString("CertFile");
            auto certStore = jsonObject->GetNamedString("CertStore");
            auto isDMSelfUpdate = jsonObject->GetNamedBoolean("IsDMSelfUpdate");

            auto d = ref new Microsoft::Devices::Management::Message::AppInstallRequestData(packageFamilyName, startUp, appxPath, depsVector, certFile, certStore, isDMSelfUpdate);
            return ref new AppInstallRequest(d);
        }

        virtual property DMMessageKind Tag {
            DMMessageKind get();
        }
    };

    public ref class AppInstallResponseData sealed
    {
    public:
        property String^ pkgFamilyName;
        property String^ name;
        property String^ version;
        property StartUpType startUp;
        property String^ installDate;
        property int errorCode;
        property String^ errorMessage;

        Blob^ Serialize(uint32_t tag) {
            JsonObject^ jsonObject = ref new JsonObject();

            jsonObject->Insert("pkgFamilyName", JsonValue::CreateStringValue(pkgFamilyName));
            jsonObject->Insert("name", JsonValue::CreateStringValue(name));
            jsonObject->Insert("version", JsonValue::CreateStringValue(version));
            jsonObject->Insert("startUp", JsonValue::CreateNumberValue(static_cast<int>(startUp)));
            jsonObject->Insert("installDate", JsonValue::CreateStringValue(installDate));
            jsonObject->Insert("errorCode", JsonValue::CreateNumberValue(errorCode));
            jsonObject->Insert("errorMessage", JsonValue::CreateStringValue(errorMessage));

            return SerializationHelper::CreateBlobFromJson(tag, jsonObject);
        }

        static AppInstallResponseData^ Deserialize(Blob^ blob) {
            String^ str = SerializationHelper::GetStringFromBlob(blob);
            JsonObject^ jsonObject = JsonObject::Parse(str);
            auto result = ref new AppInstallResponseData();

            result->pkgFamilyName = jsonObject->GetNamedString("pkgFamilyName");
            result->name = jsonObject->GetNamedString("name");
            result->version = jsonObject->GetNamedString("version");
            result->startUp = static_cast<StartUpType>(static_cast<int>(jsonObject->GetNamedNumber("startUp")));
            result->installDate = jsonObject->GetNamedString("installDate");
            result->errorCode = static_cast<int>(jsonObject->GetNamedNumber("errorCode"));
            result->errorMessage = jsonObject->GetNamedString("errorMessage");

            return result;
        }
    };

    public ref class AppInstallResponse sealed : public IResponse
    {
        StatusCodeResponse statusCodeResponse;

    public:
        property AppInstallResponseData^ data;

        AppInstallResponse(ResponseStatus status, AppInstallResponseData^ d) : statusCodeResponse(status, this->Tag)
        {
            data = d;
        }

        virtual Blob^ Serialize() {
            return data->Serialize((uint32_t)Tag);
        }

        static IDataPayload^ Deserialize(Blob^ blob) {
            AppInstallResponseData^ d = AppInstallResponseData::Deserialize(blob);
            return ref new AppInstallResponse(ResponseStatus::Success, d);
        }

        virtual property ResponseStatus Status {
            ResponseStatus get() { return statusCodeResponse.Status; }
        }

        virtual property DMMessageKind Tag {
            DMMessageKind get();
        }
    };
}}}}
