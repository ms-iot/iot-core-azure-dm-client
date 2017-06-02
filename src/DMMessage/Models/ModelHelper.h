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
    template<typename BaseClass>
    class DesiredAndReportedConfigurationHelper
    {
    public:
        typedef void(DeserializePropertiesFxn)(JsonObject^ jsonObject, BaseClass^ configObject);
        typedef void(SerializePropertiesFxn)(JsonObject^ jsonObject, BaseClass^ configObject);

        static JsonObject^ HandleDesiredAndReportedConfiguration(String^ str, BaseClass^ configObject)
        {
            JsonObject^ applyPropertiesObject = nullptr;
            if (str->Equals(ref new Platform::String(L"no-no")))
            {
                configObject->Applying = false;
                configObject->Reporting = false;
            }
            else if (str->Equals(ref new Platform::String(L"no-yes")))
            {
                configObject->Applying = false;
                configObject->Reporting = true;
            }
            else
            {
                applyPropertiesObject = JsonObject::Parse(str);
                //    "wifi": { "applyProperties": {...}|"no", "reportProperties": "yes|no" }

                configObject->Reporting = !(applyPropertiesObject->GetNamedString(ref new Platform::String(L"reportProperties")) == L"no");
                auto applyProperties = applyPropertiesObject->Lookup(ref new Platform::String(L"applyProperties"));
                if (applyProperties->ValueType == JsonValueType::Object)
                {
                    configObject->Applying = true;
                }
                else if (applyProperties->ValueType == JsonValueType::String)
                {
                    auto applyPropertiesString = applyProperties->GetString();
                    configObject->Applying = !(applyPropertiesString == L"no");
                }

            }
            return applyPropertiesObject;
        }

        static Blob^ Serialize(BaseClass^ configObject, uint32_t tag, SerializePropertiesFxn SerializeProperties) {

            JsonObject^ jsonObject = ref new JsonObject();
            if (configObject->Applying)
            {
                JsonObject^ applyPropertiesObject = ref new JsonObject();
                SerializeProperties(applyPropertiesObject, configObject);
                jsonObject->Insert("applyProperties", applyPropertiesObject);
            }
            else
            {
                jsonObject->Insert("applyProperties", JsonValue::CreateBooleanValue(false));
            }
            jsonObject->Insert("reportProperties", JsonValue::CreateBooleanValue(configObject->Reporting));

            return SerializationHelper::CreateBlobFromJson(tag, jsonObject);
        }

        static BaseClass^ Parse(Platform::String^ str, DeserializePropertiesFxn DeserializeProperties) {

            auto configObject = ref new BaseClass();
            auto jsonObject = HandleDesiredAndReportedConfiguration(str, configObject);
            if (jsonObject != nullptr)
            {
                DeserializeProperties(jsonObject, configObject);
            }
            return configObject;
        }
    };
}}}}
