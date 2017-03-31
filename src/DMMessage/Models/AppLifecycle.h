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
    public ref class AppLifecycleInfo sealed
    {
    public:
        AppLifecycleInfo()
        {
            AppId = ref new Platform::String();
            Start = true;
        }
        AppLifecycleInfo(String^ appId, bool start)
        {
            AppId = appId;
            Start = start;
        }
        property String^ AppId;
        property bool Start;
    };

    public ref class AppLifecycleRequest sealed : public IRequest
    {
    private:
        AppLifecycleInfo^ appInfo;
    
    public:
        AppLifecycleRequest(AppLifecycleInfo^ appInfo) : appInfo(appInfo) {}

        virtual Blob^ Serialize() {
            JsonObject^ jsonObject = ref new JsonObject();
            jsonObject->Insert("AppId", JsonValue::CreateStringValue(appInfo->AppId));
            jsonObject->Insert("Start", JsonValue::CreateBooleanValue(appInfo->Start));
            return SerializationHelper::CreateBlobFromJson((uint32_t)Tag, jsonObject);
        }

        static IDataPayload^ Deserialize(Blob^ bytes) {
            String^ str = SerializationHelper::GetStringFromBlob(bytes);
            JsonObject^ jsonObject = JsonObject::Parse(str);
            auto appId = jsonObject->Lookup("AppId")->GetString();
            auto start = jsonObject->Lookup("Start")->GetBoolean();
            auto appInfo = ref new Microsoft::Devices::Management::Message::AppLifecycleInfo(appId, start);
            return ref new AppLifecycleRequest(appInfo);
        }

        virtual property DMMessageKind Tag {
            DMMessageKind get() { return (appInfo->Start) ? DMMessageKind::StartApp : DMMessageKind::StopApp; };
        }

        property AppLifecycleInfo^ AppLifecycleInfo {
            Microsoft::Devices::Management::Message::AppLifecycleInfo^ get() { return appInfo; }
        }
    };
}}}}
