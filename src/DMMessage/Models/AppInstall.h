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
    public ref class AppInstallInfo sealed
    {
    public:
        AppInstallInfo()
        {
            PackageFamilyName = ref new Platform::String();
            AppxPath = ref new Platform::String();
            Dependencies = ref new Vector<String^>();
        }
        AppInstallInfo(String^ packageFamilyName, String^ appxPath, IVector<String^>^ dependencies)
        {
            PackageFamilyName = packageFamilyName;
            AppxPath = appxPath;
            Dependencies = dependencies;
        }
        property String^ PackageFamilyName;
        property String^ AppxPath;
        property IVector<String^>^ Dependencies;
    };

    public ref class AppInstallRequest sealed : public IRequest
    {
        AppInstallInfo^ appInfo;
    public:
        AppInstallRequest(AppInstallInfo^ appInfo) : appInfo(appInfo) {}

        virtual Blob^ Serialize() {
            JsonObject^ jsonObject = ref new JsonObject();
            jsonObject->Insert("PackageFamilyName", JsonValue::CreateStringValue(appInfo->PackageFamilyName));
            jsonObject->Insert("AppxPath", JsonValue::CreateStringValue(appInfo->AppxPath));
            JsonArray^ jsonArray = ref new JsonArray();
            for each (auto dep in appInfo->Dependencies)
            {
                jsonArray->Append(JsonValue::CreateStringValue(dep));
            }
            jsonObject->Insert("Dependencies", jsonArray);

            return SerializationHelper::CreateBlobFromJson((uint32_t)Tag, jsonObject);
        }

        static IDataPayload^ Deserialize(Blob^ bytes) {
            String^ str = SerializationHelper::GetStringFromBlob(bytes);
            JsonObject^ jsonObject = JsonObject::Parse(str);
            auto packageFamilyName = jsonObject->Lookup("PackageFamilyName")->GetString();
            auto appxPath = jsonObject->Lookup("AppxPath")->GetString();
            auto dependencies = jsonObject->Lookup("Dependencies")->GetArray();
            auto depsVector = ref new Vector<String^>();
            for each (auto dep in dependencies)
            {
                depsVector->Append(dep->GetString());
            }
            auto appInfo = ref new Microsoft::Devices::Management::Message::AppInstallInfo(packageFamilyName, appxPath, depsVector);
            return ref new AppInstallRequest(appInfo);
        }

        virtual property DMMessageKind Tag {
            DMMessageKind get();
        }

        property AppInstallInfo^ AppInstallInfo {
            Microsoft::Devices::Management::Message::AppInstallInfo^ get() { return appInfo; }
        }
    };
}}}}
