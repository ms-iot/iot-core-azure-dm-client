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
    public ref class StartupAppInfo sealed
    {
    public:
        StartupAppInfo()
        {
            AppId = ref new Platform::String();
            IsBackgroundApplication = false;
        }
        StartupAppInfo(String^ appId, bool start)
        {
            AppId = appId;
            IsBackgroundApplication = start;
        }
        property String^ AppId;
        property bool IsBackgroundApplication;
    };

    ref class StartupAppRequest sealed
    {
    public:
        StartupAppRequest(StartupAppInfo^ appInfo, DMMessageKind tag) 
        { 
            AppInfo = appInfo; 
            Tag = tag; 
        }

        Blob^ Serialize() {
            JsonObject^ jsonObject = ref new JsonObject();
            jsonObject->Insert("AppId", JsonValue::CreateStringValue(AppInfo->AppId));
            jsonObject->Insert("IsBackgroundApplication", JsonValue::CreateBooleanValue(AppInfo->IsBackgroundApplication));
            return SerializationHelper::CreateBlobFromJson((uint32_t)Tag, jsonObject);
        }

        static StartupAppInfo^ Deserialize(Blob^ bytes) {
            String^ str = SerializationHelper::GetStringFromBlob(bytes);
            JsonObject^ jsonObject = JsonObject::Parse(str);
            auto appId = jsonObject->Lookup("AppId")->GetString();
            auto start = jsonObject->Lookup("IsBackgroundApplication")->GetBoolean();
            return ref new Microsoft::Devices::Management::Message::StartupAppInfo(appId, start);
        }

        property StartupAppInfo^ AppInfo;
        property DMMessageKind Tag;
    };

    public ref class AddStartupAppRequest sealed : public IRequest
    {
        StartupAppRequest^ request;
    public:
        AddStartupAppRequest(StartupAppInfo^ appInfo) {
            request = ref new StartupAppRequest(appInfo, Tag);
        }

        virtual Blob^ Serialize() { return request->Serialize(); }
        static IDataPayload^ Deserialize(Blob^ bytes) {
            auto appInfo = StartupAppRequest::Deserialize(bytes);
            return ref new AddStartupAppRequest(appInfo);
        }

        virtual property DMMessageKind Tag { DMMessageKind get(); }

        property StartupAppInfo^ StartupAppInfo {
            Microsoft::Devices::Management::Message::StartupAppInfo^ get() { return request->AppInfo; }
        }
    };

    public ref class RemoveStartupAppRequest sealed : public IRequest
    {
        StartupAppRequest^ request;
    public:
        RemoveStartupAppRequest(StartupAppInfo^ appInfo) {
            request = ref new StartupAppRequest(appInfo, Tag);
        }

        virtual Blob^ Serialize() { return request->Serialize(); }
        static IDataPayload^ Deserialize(Blob^ bytes) {
            auto appInfo = StartupAppRequest::Deserialize(bytes);
            return ref new RemoveStartupAppRequest(appInfo);
        }

        virtual property DMMessageKind Tag { DMMessageKind get(); }

        property StartupAppInfo^ StartupAppInfo {
            Microsoft::Devices::Management::Message::StartupAppInfo^ get() { return request->AppInfo; }
        }
    };

    public ref class GetStartupForegroundAppResponse sealed : public IResponse
    {
        ResponseStatus status;
        String^ startupForegroundApp;
    public:
        GetStartupForegroundAppResponse(ResponseStatus status, String^ startupForegroundApp) : status(status), startupForegroundApp(startupForegroundApp) {}

        virtual Blob^ Serialize() {
            JsonObject^ jsonObject = ref new JsonObject();
            jsonObject->Insert("Status", JsonValue::CreateNumberValue((uint32_t)status));
            jsonObject->Insert("StartupForegroundApp", JsonValue::CreateStringValue(startupForegroundApp));
            return SerializationHelper::CreateBlobFromJson((uint32_t)Tag, jsonObject);
        }

        static IDataPayload^ Deserialize(Blob^ bytes) {
            auto str = SerializationHelper::GetStringFromBlob(bytes);
            auto jsonObject = JsonObject::Parse(str);
            auto status = (ResponseStatus)(uint32_t)jsonObject->Lookup("Status")->GetNumber();
            auto startupForegroundApp = jsonObject->Lookup("StartupForegroundApp")->GetString();
            return ref new GetStartupForegroundAppResponse(status, startupForegroundApp);
        }

        virtual property DMMessageKind Tag {
            DMMessageKind get();
        }

        virtual property ResponseStatus Status {
            ResponseStatus get() { return status; }
        }

        property String^ StartupForegroundApp {
            String^ get() { return startupForegroundApp; }
        }
    };

    public ref class ListStartupBackgroundAppsResponse sealed : public IResponse
    {
    private:
        ResponseStatus status;
        IVector<String^>^ startupBackgroundApps;

    public:
        ListStartupBackgroundAppsResponse(ResponseStatus status, IVector<String^>^ startupBackgroundApps) : status(status), startupBackgroundApps(startupBackgroundApps) {}

    internal:
        ListStartupBackgroundAppsResponse(ResponseStatus status, JsonArray^ appsJson) : status(status) 
        {
            startupBackgroundApps = ref new Vector<String^>();
            for each (auto app in appsJson)
            {
                startupBackgroundApps->Append(app->GetString());
            }
        }

    public:
        virtual Blob^ Serialize() {
            auto jsonObject = ref new JsonObject();
            jsonObject->Insert("Status", JsonValue::CreateNumberValue((uint32_t)status));
            auto jsonArray = ref new JsonArray();
            for (unsigned int i=0; i<startupBackgroundApps->Size; i++)
            {
                jsonArray->Append(JsonValue::CreateStringValue(startupBackgroundApps->GetAt(i)));
            }
            jsonObject->Insert("StartupBackgroundApps", jsonArray);
            return SerializationHelper::CreateBlobFromJson((uint32_t)Tag, jsonObject);
        }

        static IDataPayload^ Deserialize(Blob^ bytes) {
            auto str = SerializationHelper::GetStringFromBlob(bytes);
            auto jsonObject = JsonObject::Parse(str);
            auto status = (ResponseStatus)(uint32_t)jsonObject->Lookup("Status")->GetNumber();
            auto jsonArray = jsonObject->Lookup("StartupBackgroundApps")->GetArray();
            return ref new ListStartupBackgroundAppsResponse(status, jsonArray);
        }

        virtual property DMMessageKind Tag {
            DMMessageKind get();
        }

        virtual property ResponseStatus Status {
            ResponseStatus get() { return status; }
        }

        property IVector<String^>^ StartupBackgroundApps {
            IVector<String^>^ get() { return startupBackgroundApps; }
        }
    };

}}}}
