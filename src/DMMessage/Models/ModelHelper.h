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
#include "Blob.h"

#undef GetObject

namespace Microsoft { namespace Devices { namespace Management { namespace Message
{

static String^ JsonYes = L"yes";
static String^ JsonNo = L"no";
static String^ JsonApplyProperties = L"applyProperties";
static String^ JsonReportProperties = L"reportProperties";
static String^ JsonNoApplyNoReport = L"no-apply-no-report";
static String^ JsonNoApplyYesReport = L"no-apply-yes-report";

template<typename BaseClass>
class DeviceTwinDesiredConfiguration
{
public:
    typedef void(DeserializePropertiesFxn)(JsonObject^ jApplyProperties, BaseClass^ configObject);
    typedef void(SerializePropertiesFxn)(JsonObject^ jApplyProperties, BaseClass^ configObject);

    // Reads the DeviceTwin configuration portion (ApplyFromDeviceTwin and ReportToDeviceTwin), and applies them to the configObject.
    // Returns the inner applyProperties object if it exists.
    static JsonObject^ HandleDeviceTwinConfiguration(String^ configObjectString, BaseClass^ configObject)
    {
        JsonObject^ jApplyProperties = nullptr;

        if (configObjectString->Equals(JsonNoApplyNoReport))
        {
            configObject->ApplyFromDeviceTwin = JsonNo;
            configObject->ReportToDeviceTwin = JsonNo;
        }
        else if (configObjectString->Equals(JsonNoApplyYesReport))
        {
            configObject->ApplyFromDeviceTwin = JsonNo;
            configObject->ReportToDeviceTwin = JsonYes;
        }
        else
        {
            JsonObject^ jConfigObject = JsonObject::Parse(configObjectString);

            if (jConfigObject->HasKey(JsonApplyProperties))
            {
                IJsonValue^ jValue = jConfigObject->Lookup(JsonApplyProperties);
                if (jValue->ValueType == JsonValueType::Object)
                {
                    jApplyProperties = jValue->GetObject();
                    configObject->ApplyFromDeviceTwin = JsonYes;
                }
                else if (jValue->ValueType == JsonValueType::String)
                {
                    configObject->ApplyFromDeviceTwin = jValue->GetString();
                }
            }

            if (jConfigObject->HasKey(JsonReportProperties))
            {
                configObject->ReportToDeviceTwin = jConfigObject->GetNamedString(JsonReportProperties);
            }
        }

        return jApplyProperties;
    }

    static Blob^ Serialize(BaseClass^ configObject, uint32_t tag, SerializePropertiesFxn SerializeProperties)
    {
        JsonObject^ jConfigObject = ref new JsonObject();
        if (configObject->ApplyFromDeviceTwin == JsonYes)
        {
            JsonObject^ jApplyProperties = ref new JsonObject();
            SerializeProperties(jApplyProperties, configObject);
            jConfigObject->Insert(JsonApplyProperties, jApplyProperties);
        }
        else
        {
            jConfigObject->Insert(JsonApplyProperties, JsonValue::CreateStringValue(JsonNo));
        }
        jConfigObject->Insert(JsonReportProperties, JsonValue::CreateStringValue(configObject->ReportToDeviceTwin));

        return SerializationHelper::CreateBlobFromJson(tag, jConfigObject);
    }

    static BaseClass^ Deserialize(Platform::String^ configObjectString, DeserializePropertiesFxn DeserializeProperties)
    {
        auto configObject = ref new BaseClass();
        auto jApplyProperties = HandleDeviceTwinConfiguration(configObjectString, configObject);
        if (jApplyProperties != nullptr)
        {
            DeserializeProperties(jApplyProperties, configObject);
        }

        return configObject;
    }
};

template<typename BaseClass>
class DeviceTwinReportedConfiguration
{
public:
    typedef void(DeserializePropertiesFxn)(JsonObject^ jReportProperties, BaseClass^ configObject);
    typedef void(SerializePropertiesFxn)(JsonObject^ jReportProperties, BaseClass^ configObject);

    // Reads the DeviceTwin configuration portion (ReportToDeviceTwin), and applies it to the configObject.
    // Returns the inner reportProperties object if it exists.
    static JsonObject^ HandleDeviceTwinConfiguration(String^ configObjectString, BaseClass^ configObject)
    {
        JsonObject^ jReportProperties = nullptr;

        if (configObjectString->Equals(JsonYes))
        {
            configObject->ReportToDeviceTwin = JsonYes;
        }
        else
        {
            JsonObject^ jConfigObject = JsonObject::Parse(configObjectString);

            if (jConfigObject->HasKey(JsonReportProperties))
            {
                IJsonValue^ jValue = jConfigObject->Lookup(JsonReportProperties);
                if (jValue->ValueType == JsonValueType::Object)
                {
                    jReportProperties = jValue->GetObject();
                    configObject->ReportToDeviceTwin = JsonYes;
                }
                else if (jValue->ValueType == JsonValueType::String)
                {
                    configObject->ReportToDeviceTwin = jValue->GetString();
                }
            }
        }

        return jReportProperties;
    }

    static Blob^ Serialize(BaseClass^ configObject, uint32_t tag, SerializePropertiesFxn SerializeProperties)
    {
        JsonObject^ jConfigObject = ref new JsonObject();
        if (configObject->ReportToDeviceTwin == JsonYes)
        {
            JsonObject^ jReportProperties = ref new JsonObject();
            SerializeProperties(jReportProperties, configObject);
            jConfigObject->Insert(JsonReportProperties, jReportProperties);
        }
        else
        {
            jConfigObject->Insert(JsonReportProperties, JsonValue::CreateStringValue(JsonNo));
        }

        return SerializationHelper::CreateBlobFromJson(tag, jConfigObject);
    }

    static BaseClass^ Deserialize(Platform::String^ configObjectString, DeserializePropertiesFxn DeserializeProperties)
    {
        auto configObject = ref new BaseClass();
        auto jReportProperties = HandleDeviceTwinConfiguration(configObjectString, configObject);
        if (jReportProperties != nullptr)
        {
            DeserializeProperties(jReportProperties, configObject);
        }

        return configObject;
    }
};
}}}}
