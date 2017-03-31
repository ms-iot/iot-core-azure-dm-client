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
    public ref class GetWindowsUpdatesConfiguration sealed
    {
    public:
        property String^ installed;
        property String^ approved;
        property String^ failed;
        property String^ installable;
        property String^ pendingReboot;
        property String^ lastScanTime;
        property bool    deferUpgrade;

        GetWindowsUpdatesConfiguration()
        {
            deferUpgrade = false;
        }

        Blob^ Serialize(uint32_t tag) {
            JsonObject^ jsonObject = ref new JsonObject();

            jsonObject->Insert("installed", JsonValue::CreateStringValue(installed));
            jsonObject->Insert("approved", JsonValue::CreateStringValue(approved));
            jsonObject->Insert("failed", JsonValue::CreateStringValue(failed));
            jsonObject->Insert("installable", JsonValue::CreateStringValue(installable));
            jsonObject->Insert("pendingReboot", JsonValue::CreateStringValue(pendingReboot));
            jsonObject->Insert("lastScanTime", JsonValue::CreateStringValue(lastScanTime));
            jsonObject->Insert("deferUpgrade", JsonValue::CreateBooleanValue(deferUpgrade));

            return SerializationHelper::CreateBlobFromJson(tag, jsonObject);
        }

        static GetWindowsUpdatesConfiguration^ Deserialize(Blob^ blob) {
            String^ str = SerializationHelper::GetStringFromBlob(blob);
            JsonObject^ jsonObject = JsonObject::Parse(str);
            auto result = ref new GetWindowsUpdatesConfiguration();

            result->installed = jsonObject->Lookup("installed")->GetString();
            result->approved = jsonObject->Lookup("approved")->GetString();
            result->failed = jsonObject->Lookup("failed")->GetString();
            result->installable = jsonObject->Lookup("installable")->GetString();
            result->pendingReboot = jsonObject->Lookup("pendingReboot")->GetString();
            result->lastScanTime = jsonObject->Lookup("lastScanTime")->GetString();
            result->deferUpgrade = jsonObject->Lookup("deferUpgrade")->GetBoolean();

            return result;
        }
    };

    public ref class GetWindowsUpdatesRequest sealed : public IRequest
    {
    public:
        virtual Blob^ Serialize() {
            return SerializationHelper::CreateEmptyBlob((uint32_t)Tag);
        }

        static IDataPayload^ Deserialize(Blob^ bytes) {
            return ref new GetWindowsUpdatesRequest();
        }

        virtual property DMMessageKind Tag {
            DMMessageKind get();
        }
    };

    public ref class GetWindowsUpdatesResponse sealed : public IResponse
    {
        StatusCodeResponse statusCodeResponse;

    public:
        property GetWindowsUpdatesConfiguration^ configuration;

        GetWindowsUpdatesResponse(ResponseStatus status, GetWindowsUpdatesConfiguration^ updatesConfiguration) : statusCodeResponse(status, this->Tag)
        {
            configuration = updatesConfiguration;
        }

        virtual Blob^ Serialize() {
            return configuration->Serialize((uint32_t)Tag);
        }

        static IDataPayload^ Deserialize(Blob^ blob) {
            GetWindowsUpdatesConfiguration^ updatesConfiguration = GetWindowsUpdatesConfiguration::Deserialize(blob);
            return ref new GetWindowsUpdatesResponse(ResponseStatus::Success, updatesConfiguration);
        }

        virtual property ResponseStatus Status {
            ResponseStatus get() { return statusCodeResponse.Status; }
        }

        virtual property DMMessageKind Tag {
            DMMessageKind get();
        }
    };

    public ref class SetWindowsUpdatesConfiguration sealed
    {
    public:
        property String^ approved;

        Blob^ Serialize(uint32_t tag) {
            JsonObject^ jsonObject = ref new JsonObject();
            jsonObject->Insert("approved", JsonValue::CreateStringValue(approved));
            return SerializationHelper::CreateBlobFromJson(tag, jsonObject);
        }

        static SetWindowsUpdatesConfiguration^ Deserialize(Blob^ blob) {
            String^ str = SerializationHelper::GetStringFromBlob(blob);

            JsonObject^ jsonObject = JsonObject::Parse(str);
            auto result = ref new SetWindowsUpdatesConfiguration();
            result->approved = jsonObject->Lookup("approved")->GetString();
            return result;
        }
    };

    public ref class SetWindowsUpdatesRequest sealed : public IRequest
    {
    public:
        property SetWindowsUpdatesConfiguration^ configuration;

        SetWindowsUpdatesRequest(SetWindowsUpdatesConfiguration^ setWindowsUpdatesConfiguration)
        {
            configuration = setWindowsUpdatesConfiguration;
        }

        virtual Blob^ Serialize() {
            return configuration->Serialize((uint32_t)Tag);
        }

        static IDataPayload^ Deserialize(Blob^ blob) {
            auto policyConfiguration = SetWindowsUpdatesConfiguration::Deserialize(blob);
            return ref new SetWindowsUpdatesRequest(policyConfiguration);
        }

        virtual property DMMessageKind Tag {
            DMMessageKind get();
        }
    };
}
}}}

