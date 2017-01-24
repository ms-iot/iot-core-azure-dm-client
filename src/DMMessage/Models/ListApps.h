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
    public ref class AppInfo sealed
    {
    public:
        property String^ PackageFamilyName;
    };

    public ref class ListAppsResponse sealed : public IResponse
    {
        ResponseStatus status;
        IVector<AppInfo^>^ apps;
    public:
        ListAppsResponse(ResponseStatus status, IVector<AppInfo^>^ apps) : status(status), apps(apps) {}

        virtual Blob^ Serialize() {
            auto jsonObject = ref new JsonObject();
            jsonObject->Insert("Status", JsonValue::CreateNumberValue((uint32_t)status));
            auto jsonArray = ref new JsonArray();
            for each (auto app in apps) 
            {
                auto jsonApp = ref new JsonObject();
                jsonApp->Insert("PackageFamilyName", JsonValue::CreateStringValue(app->PackageFamilyName));
                jsonArray->Append(jsonApp);
            }
            jsonObject->Insert("Apps", jsonArray);
            return SerializationHelper::CreateBlobFromJson((uint32_t)Tag, jsonObject);
        }

        static IDataPayload^ Deserialize(Blob^ bytes) {
            auto str = SerializationHelper::GetStringFromBlob(bytes);
            auto jsonObject = JsonObject::Parse(str);
            auto status = (ResponseStatus)(uint32_t)jsonObject->GetNamedNumber("Status");
            auto appArray = jsonObject->GetNamedArray("Apps");
            auto appInfoVector = ref new Vector<AppInfo^>();
            for (unsigned int i=0; i<appArray->Size; i++)
            {
                auto appObject = appArray->GetObjectAt(i);
                auto appInfo = ref new AppInfo();
                appInfo->PackageFamilyName = appObject->Lookup("PackageFamilyName")->GetString();
                appInfoVector->Append(appInfo);
            }
            return ref new ListAppsResponse(status, appInfoVector);
        }

        virtual property DMMessageKind Tag {
            DMMessageKind get();
        }

        virtual property ResponseStatus Status {
            ResponseStatus get() { return status; }
        }

        property IVector<AppInfo^>^ Apps {
            IVector<AppInfo^>^ get() { return apps; }
        }
    };
}}}}
