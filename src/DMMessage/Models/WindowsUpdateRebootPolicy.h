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
    public ref class WindowsUpdateRebootPolicyConfiguration sealed
    {
    public:
        property bool allow;

        WindowsUpdateRebootPolicyConfiguration()
        {
            allow = true;
        }

        Blob^ Serialize(uint32_t tag)
        {
            JsonObject^ jsonObject = ref new JsonObject();
            jsonObject->Insert("allow", JsonValue::CreateBooleanValue(allow));
            return SerializationHelper::CreateBlobFromJson(tag, jsonObject);
        }

        static WindowsUpdateRebootPolicyConfiguration^ Deserialize(Blob^ blob)
        {
            String^ str = SerializationHelper::GetStringFromBlob(blob);
            JsonObject^ jsonObject = JsonObject::Parse(str);

            auto configuration = ref new WindowsUpdateRebootPolicyConfiguration();
            configuration->allow = jsonObject->Lookup("allow")->GetBoolean();
            return configuration;
        }
    };

    public ref class SetWindowsUpdateRebootPolicyRequest sealed : public IRequest
    {
    public:
        property WindowsUpdateRebootPolicyConfiguration^ configuration;

        SetWindowsUpdateRebootPolicyRequest(WindowsUpdateRebootPolicyConfiguration^ windowsUpdatePolicyConfiguration)
        {
            configuration = windowsUpdatePolicyConfiguration;
        }

        virtual Blob^ Serialize()
        {
            return configuration->Serialize((uint32_t)Tag);
        }

        static IDataPayload^ Deserialize(Blob^ blob)
        {
            WindowsUpdateRebootPolicyConfiguration^ policyConfiguration = WindowsUpdateRebootPolicyConfiguration::Deserialize(blob);
            return ref new SetWindowsUpdateRebootPolicyRequest(policyConfiguration);
        }

        virtual property DMMessageKind Tag {
            DMMessageKind get();
        }
    };

    public ref class GetWindowsUpdateRebootPolicyRequest sealed : public IRequest
    {
    public:
        GetWindowsUpdateRebootPolicyRequest() {}

        virtual Blob^ Serialize()
        {
            return SerializationHelper::CreateEmptyBlob((uint32_t)Tag);
        }

        static IDataPayload^ Deserialize(Blob^ blob)
        {
            assert(blob->Tag == DMMessageKind::GetWindowsUpdateRebootPolicy);
            return ref new GetWindowsUpdateRebootPolicyRequest();
        }

        virtual property DMMessageKind Tag
        {
            DMMessageKind get();
        }
    };

    public ref class GetWindowsUpdateRebootPolicyResponse sealed : public IResponse
    {
        StatusCodeResponse statusCodeResponse;
    public:
        property WindowsUpdateRebootPolicyConfiguration^ configuration;

        GetWindowsUpdateRebootPolicyResponse(ResponseStatus status, WindowsUpdateRebootPolicyConfiguration^ updateConfiguration) : statusCodeResponse(status, this->Tag)
        {
            configuration = updateConfiguration;
        }

        virtual Blob^ Serialize()
        {
            return configuration->Serialize((uint32_t)Tag);
        }

        static IDataPayload^ Deserialize(Blob^ blob)
        {
            auto policyConfiguration = WindowsUpdateRebootPolicyConfiguration::Deserialize(blob);
            return ref new GetWindowsUpdateRebootPolicyResponse(ResponseStatus::Success, policyConfiguration);
        }

        virtual property ResponseStatus Status
        {
            ResponseStatus get() { return statusCodeResponse.Status; }
        }

        virtual property DMMessageKind Tag
        {
            DMMessageKind get();
        }
    };
}
}}}
