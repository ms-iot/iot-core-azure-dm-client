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
#include <collection.h>
#include "IRequestIResponse.h"
#include "SerializationHelper.h"
#include "DMMessageKind.h"
#include "StatusCodeResponse.h"
#include "Blob.h"

using namespace Platform;
using namespace Platform::Collections;
using namespace Platform::Metadata;
using namespace Windows::Data::Json;
using namespace Windows::Foundation::Collections;

#ifdef GetObject
#undef GetObject
#endif

namespace Microsoft { namespace Devices { namespace Management { namespace Message
{
    public enum class PolicySource
    {
        Local,
        Remote,
        Unknown
    };

    [Windows::Foundation::Metadata::WebHostHidden]
    public ref class Policy sealed
    {
    public:
        property PolicySource source;
        property IVector<PolicySource>^ sourcePriorities;

        void Serialize(JsonObject^ targetObj)
        {
            JsonObject^ jsonObject = ref new JsonObject();
            jsonObject->Insert("source", JsonValue::CreateNumberValue(static_cast<int>(source)));
            JsonArray^ jsonArray = ref new JsonArray();
            for (unsigned int i = 0; i < sourcePriorities->Size; ++i)
            {
                PolicySource priority = sourcePriorities->GetAt(i);
                jsonArray->Append(JsonValue::CreateNumberValue(static_cast<int>(priority)));
            }
            jsonObject->Insert("sourcePriorities", jsonArray);

            targetObj->Insert("policy", jsonObject);
        }

        static Policy^ Deserialize(JsonObject^ sourceObj)
        {
            if (!sourceObj->HasKey("policy"))
            {
                return nullptr;
            }

            IJsonValue^ jsonPolicy = sourceObj->Lookup("policy"); 
            JsonObject^ jsonPolicyObj = jsonPolicy->GetObject();
            double src = jsonPolicyObj->Lookup("source")->GetNumber();
            JsonArray^ srcPriorities = jsonPolicyObj->Lookup("sourcePriorities")->GetArray();

            Policy^ policy = ref new Policy();
            policy->source = static_cast<PolicySource>(static_cast<int>(src));
            policy->sourcePriorities = ref new Vector<PolicySource>();
            for (unsigned int i = 0; i < srcPriorities->Size; ++i)
            {
                policy->sourcePriorities->Append(static_cast<PolicySource>(static_cast<int>(srcPriorities->GetNumberAt(i))));
            }

            return policy;
        }
    };
}
}}}
