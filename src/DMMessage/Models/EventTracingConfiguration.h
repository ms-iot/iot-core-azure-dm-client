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
#include <string>
#include "IRequestIResponse.h"
#include "SerializationHelper.h"
#include "DMMessageKind.h"
#include "StatusCodeResponse.h"
#include "Blob.h"
#include "ModelHelper.h"

using namespace Platform;
using namespace Platform::Metadata;
using namespace Windows::Data::Json;
using namespace Windows::Foundation::Collections;

namespace Microsoft { namespace Devices { namespace Management { namespace Message
{
    [Windows::Foundation::Metadata::WebHostHidden]
    public ref class ProviderConfiguration sealed
    {
    public:
        property String^ Guid;
        property String^ TraceLevel;
        property String^ Keywords;
        property bool Enabled;

        JsonObject^ ToJsonObject(JsonObject^ targetJsonObject)
        {
            targetJsonObject->Insert("traceLevel", JsonValue::CreateStringValue(TraceLevel));
            targetJsonObject->Insert("keywords", JsonValue::CreateStringValue(Keywords));
            targetJsonObject->Insert("enabled", JsonValue::CreateBooleanValue(Enabled));
            targetJsonObject->Insert("type", JsonValue::CreateStringValue(L"provider"));
            return targetJsonObject;
        }

        static ProviderConfiguration^ FromJsonObject(String^ name, JsonObject^ jsonObject)
        {
            ProviderConfiguration^ configuration = ref new ProviderConfiguration();
            configuration->Guid = name;
            configuration->TraceLevel = jsonObject->GetNamedString(L"traceLevel");
            configuration->Keywords = jsonObject->GetNamedString(L"keywords");
            configuration->Enabled = jsonObject->GetNamedBoolean(L"enabled");
            return configuration;
        }
    };

    [Windows::Foundation::Metadata::WebHostHidden]
    public ref class CollectorCSPConfiguration sealed
    {
    public:
        property String^ TraceLogFileMode;
        property int LogFileSizeLimitMB;
        property String^ LogFileFolder;
        property String^ Started;
        property IVector<ProviderConfiguration^>^ Providers;

        CollectorCSPConfiguration()
        {
            Providers = ref new Vector<ProviderConfiguration^>();
        }

        JsonObject^ ToJsonObject(JsonObject^ targetJsonObject)
        {
            targetJsonObject->Insert("traceLogFileMode", JsonValue::CreateStringValue(TraceLogFileMode));
            targetJsonObject->Insert("logFileSizeLimitMB", JsonValue::CreateNumberValue(LogFileSizeLimitMB));
            targetJsonObject->Insert("logFileFolder", JsonValue::CreateStringValue(LogFileFolder));
            targetJsonObject->Insert("started", JsonValue::CreateStringValue(Started));

            for each (ProviderConfiguration^ provider in Providers)
            {
                JsonObject^ jsonObjectProperties = ref new JsonObject();
                provider->ToJsonObject(jsonObjectProperties);
                targetJsonObject->Insert(provider->Guid, jsonObjectProperties);
            }

            return targetJsonObject;
        }

        static CollectorCSPConfiguration^ FromJsonObject(JsonObject^ applyPropertiesObject)
        {
            auto configuration = ref new CollectorCSPConfiguration();

            IIterator<IKeyValuePair<String^, IJsonValue^>^>^ it = applyPropertiesObject->First();
            while (it->HasCurrent)
            {
                if (it->Current->Key == L"traceLogFileMode")
                {
                    configuration->TraceLogFileMode = it->Current->Value->GetString();
                }
                else if (it->Current->Key == L"logFileSizeLimitMB")
                {
                    configuration->LogFileSizeLimitMB = static_cast<int>(it->Current->Value->GetNumber());
                }
                else if (it->Current->Key == L"logFileFolder")
                {
                    configuration->LogFileFolder = it->Current->Value->GetString();
                }
                else if (it->Current->Key == L"started")
                {
                    configuration->Started = it->Current->Value->GetString();
                }
                else
                {
                    if (it->Current->Value->ValueType == JsonValueType::Object)
                    {
                        JsonObject^ jObject = it->Current->Value->GetObject();
                        if (jObject->GetNamedString(L"type") == L"provider")
                        {
                            ProviderConfiguration^ provider = ProviderConfiguration::FromJsonObject(it->Current->Key, jObject);
                            configuration->Providers->Append(provider);
                        }
                    }
                }

                if (!it->MoveNext())
                {
                    break;
                }
            }
            return configuration;
        }
    };

    [Windows::Foundation::Metadata::WebHostHidden]
    public ref class CollectorDesiredConfiguration sealed
    {
    public:
        property String^ Name;
        property String^ ReportToDeviceTwin;
        property String^ ApplyFromDeviceTwin;
        property CollectorCSPConfiguration^ CSPConfiguration;

        JsonObject^ ToJsonObject()
        {
            return DeviceTwinDesiredConfiguration<CollectorDesiredConfiguration>::ToJson(this, [](JsonObject^ applyPropertiesObject, CollectorDesiredConfiguration^ configObject)
            {
                configObject->CSPConfiguration->ToJsonObject(applyPropertiesObject);
            });
        }

        static CollectorDesiredConfiguration^ FromJsonObject(String^ name, IJsonValue^ jsonValue)
        {
            CollectorDesiredConfiguration^ configuration = DeviceTwinDesiredConfiguration<CollectorDesiredConfiguration>::Deserialize(jsonValue->Stringify(), [](JsonObject^ applyPropertiesObject, CollectorDesiredConfiguration^ configObject)
            {
                configObject->CSPConfiguration = CollectorCSPConfiguration::FromJsonObject(applyPropertiesObject);
            });
            configuration->Name = name;
            return configuration;
        }
    };

    [Windows::Foundation::Metadata::WebHostHidden]
    public ref class SetEventTracingConfigurationRequest sealed : public IRequest
    {
    public:
        property IVector<CollectorDesiredConfiguration^>^ Collectors;

        SetEventTracingConfigurationRequest()
        {
            Collectors = ref new Vector<CollectorDesiredConfiguration^>();
        }

        virtual Blob^ Serialize()
        {
            JsonObject^ jsonObject = ref new JsonObject();
            for each (CollectorDesiredConfiguration^ collector in Collectors)
            {
                JsonObject^ jsonObjectProperties = collector->ToJsonObject();
                jsonObject->Insert(collector->Name, jsonObjectProperties);
            }
            return SerializationHelper::CreateBlobFromJson((uint32_t)Tag, jsonObject);
        }

        static IDataPayload^ Deserialize(Blob^ blob)
        {
            SetEventTracingConfigurationRequest^ request = ref new SetEventTracingConfigurationRequest();
            String^ str = SerializationHelper::GetStringFromBlob(blob);
            JsonObject^ jsonObject = JsonObject::Parse(str);

            IIterator<IKeyValuePair<String^, IJsonValue^>^>^ it = jsonObject->First();
            while (it->HasCurrent)
            {
                CollectorDesiredConfiguration^ collectorConfiguration = CollectorDesiredConfiguration::FromJsonObject(it->Current->Key, it->Current->Value);
                request->Collectors->Append(collectorConfiguration);
                if (!it->MoveNext())
                {
                    break;
                }
            }

            return request;
        }

        virtual property DMMessageKind Tag
        {
            DMMessageKind get();
        }
    };

    [Windows::Foundation::Metadata::WebHostHidden]
    public ref class GetEventTracingConfigurationRequest sealed : public IRequest
    {
    public:
        GetEventTracingConfigurationRequest() {}

        virtual Blob^ Serialize()
        {
            return SerializationHelper::CreateEmptyBlob((uint32_t)Tag);
        }

        static IDataPayload^ Deserialize(Blob^ blob)
        {
            return ref new GetEventTracingConfigurationRequest();
        }

        virtual property DMMessageKind Tag
        {
            DMMessageKind get();
        }
    };

    [Windows::Foundation::Metadata::WebHostHidden]
    public ref class CollectorReportedConfiguration sealed
    {
    public:
        property String^ Name;
        property String^ ReportToDeviceTwin;
        property CollectorCSPConfiguration^ CSPConfiguration;

        CollectorReportedConfiguration()
        {
            CSPConfiguration = ref new CollectorCSPConfiguration();
        }

        JsonObject^ ToJsonObject()
        {
            JsonObject^ jConfigObject = ref new JsonObject();
            CSPConfiguration->ToJsonObject(jConfigObject);
            jConfigObject->Insert(L"reportToDeviceTwin", JsonValue::CreateStringValue(ReportToDeviceTwin));
            return jConfigObject;
        }

        static CollectorReportedConfiguration^ FromJsonObject(String^ name, IJsonValue^ jsonValue)
        {
            auto configObject = ref new CollectorReportedConfiguration();
            configObject->Name = name;
            if (jsonValue->ValueType == JsonValueType::Object)
            {
                JsonObject^ jsonObject = jsonValue->GetObject();
                configObject->CSPConfiguration = CollectorCSPConfiguration::FromJsonObject(jsonObject);
                configObject->ReportToDeviceTwin = jsonObject->Lookup(L"reportToDeviceTwin")->GetString();
            }
            return configObject;
        }
    };

    [Windows::Foundation::Metadata::WebHostHidden]
    public ref class GetEventTracingConfigurationResponse sealed : public IResponse
    {
        StatusCodeResponse statusCodeResponse;
    public:
        property IVector<CollectorReportedConfiguration^>^ Collectors;

        GetEventTracingConfigurationResponse(ResponseStatus status) : statusCodeResponse(status, this->Tag)
        {
            Collectors = ref new Vector<CollectorReportedConfiguration^>();
        }

        JsonObject^ ToJsonObject()
        {
            JsonObject^ jsonObject = ref new JsonObject();
            for each (CollectorReportedConfiguration^ collector in Collectors)
            {
                JsonObject^ jsonObjectProperties = collector->ToJsonObject();
                jsonObject->Insert(collector->Name, jsonObjectProperties);
            }
            return jsonObject;
        }

        String^ ToJsonString()
        {
            return ToJsonObject()->Stringify();
        }

        virtual Blob^ Serialize()
        {
            return SerializationHelper::CreateBlobFromJson((uint32_t)Tag, ToJsonObject());
        }

        static IDataPayload^ Deserialize(Blob^ blob)
        {
            auto response = ref new GetEventTracingConfigurationResponse(ResponseStatus::Success);
            String^ str = SerializationHelper::GetStringFromBlob(blob);
            JsonObject^ jsonObject = JsonObject::Parse(str);

            IIterator<IKeyValuePair<String^, IJsonValue^>^>^ it = jsonObject->First();
            while (it->HasCurrent)
            {
                CollectorReportedConfiguration^ collectorConfiguration = CollectorReportedConfiguration::FromJsonObject(it->Current->Key, it->Current->Value);
                response->Collectors->Append(collectorConfiguration);
                if (!it->MoveNext())
                {
                    break;
                }
            }
            return response;
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
