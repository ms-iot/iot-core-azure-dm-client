#include "stdafx.h"

#include "AzureProxy.h"
#include "serializer.h"
#include "iothub_client.h"
#include "iothubtransportmqtt.h"
#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/threadapi.h"

#include "LocalMachine\LocalMachine.h"
#include "LocalMachine\CSPs\RebootCSP.h"

using namespace Windows::Data::Json;
using namespace Windows::Foundation::Collections;
using namespace Platform;
using namespace std;

#define JsonDesiredNode L"desired"

const char* ReportMethod = "Report";
const char* RebootMethod = "Reboot";

AzureProxy::AzureProxy(const string& connectionString) :
    _iotHubClientHandle(nullptr)
{
    TRACE("AzureProxy::ctor()");

    // Prepare the platform
    if (platform_init() != 0)
    {
        throw DMException("Failed to initialize the platform.");
    }

    // Create an IoTHub client
    _iotHubClientHandle = IoTHubClient_CreateFromConnectionString(connectionString.c_str(), MQTT_Protocol);
    if (_iotHubClientHandle == NULL)
    {
        throw DMException("Failure creating IoTHubClient handle.");
    }

    // Turn on Log 
    bool trace = false;
    IoTHubClient_SetOption(_iotHubClientHandle, "logtrace", &trace);

    // Set the desired properties callback
    if (IoTHubClient_SetDeviceTwinCallback(_iotHubClientHandle, OnDesiredProperties, this) != IOTHUB_CLIENT_OK)
    {
        throw DMException("Unable to set device twin callback.");
    }

    // Set the message properties callback
    if (IoTHubClient_SetMessageCallback(_iotHubClientHandle, OnMessageReceived, this) != IOTHUB_CLIENT_OK)
    {
        throw DMException("Unable to set message callback.");
    }

    if (IoTHubClient_SetDeviceMethodCallback(_iotHubClientHandle, OnMethodCalled, this) != IOTHUB_CLIENT_OK)
    {
        throw DMException("Unable to set method callback.");
    }
}

AzureProxy::~AzureProxy()
{
    TRACE("AzureProxy::~AzureProxy()");
    if (_iotHubClientHandle != nullptr)
    {
        IoTHubClient_Destroy(_iotHubClientHandle);
        platform_deinit();
    }
}

int AzureProxy::OnMethodCalled(const char* method_name, const unsigned char* payload, size_t size, unsigned char** response, size_t* resp_size, void* userContextCallback)
{
    AzureProxy* pThis = static_cast<AzureProxy*>(userContextCallback);
    assert(pThis);

    string methodName = method_name;
    string methodPayload(reinterpret_cast<const char*>(payload), size);
    string methodResponse;

    int retCode = pThis->ProcessMethodCall(methodName, methodPayload, methodResponse);
    if (retCode == IOTHUB_CLIENT_IOTHUB_METHOD_STATUS_ERROR)
    {
        return retCode;
    }

    if (resp_size != nullptr)
    {
        *resp_size = 0;
        if (response != nullptr)
        {
            *response = nullptr;

            if (methodResponse.size())
            {
                // The caller is responsible for freeing this malloc.
                *response = (unsigned char*)malloc(methodResponse.size());
                memcpy(*response, methodResponse.c_str(), methodResponse.size());
                *resp_size = methodResponse.size();
            }
        }
    }

    return retCode;
}

int AzureProxy::ProcessMethodCall(const string& name, const string& payload, string& response)
{
    int result = IOTHUB_CLIENT_IOTHUB_METHOD_STATUS_SUCCESS;

    try
    {
        if (name == ReportMethod)
        {
            ReportAllProperties();
        }
        else if (name == RebootMethod)
        {
            _rebootModel.ExecRebootNow();
            ReportProperties(_rebootModel.GetReportedProperties());
        }
    }
    catch (exception&)
    {
        result = IOTHUB_CLIENT_IOTHUB_METHOD_STATUS_ERROR;
    }

    return result;
}

// ToDo: Remove OnMessageReceived(). It is just a placeholder in case we need it.
IOTHUBMESSAGE_DISPOSITION_RESULT AzureProxy::OnMessageReceived(IOTHUB_MESSAGE_HANDLE message, void* userContextCallback)
{
    TRACE("AzureProxy::OnMessageReceived()");

    AzureProxy* pThis = static_cast<AzureProxy*>(userContextCallback);
    assert(pThis);

    const char* buffer;
    size_t bufferSize;

    // ToDo: who's responsible for freeing the buffer?
    if (IoTHubMessage_GetByteArray(message, (const unsigned char**)&buffer, &bufferSize) != IOTHUB_MESSAGE_OK)
    {
        TRACE("Warning: Unable to retrieve the message data.");
    }
    else
    {
        // Need to add a null terminator.
        string message(buffer, bufferSize);
        TRACEP("Received Message with Data:", message.c_str());
    }

    return IOTHUBMESSAGE_ACCEPTED;
}

void AzureProxy::OnReportedPropertiesSent(int status_code, void* userContextCallback)
{
    TRACE("AzureProxy::OnReportedPropertiesSent()");

    // AzureProxy* pThis = static_cast<AzureProxy*>(userContextCallback);
    // assert(pThis);

    // ToDo: Log a message to the event log in case of failure code.
    TRACEP("IoTHub: reported properties delivered with status_code :", status_code);
}

IJsonValue^ AzureProxy::GetDesiredPropertiesNode(DEVICE_TWIN_UPDATE_STATE update_state, const string& allJson)
{
    TRACE(L"AzureProxy::GetDesiredPropertiesNode()");
    wstring wideJsonString = Utils::MultibyteToWide(allJson.c_str());

    JsonValue^ value;
    if (!JsonValue::TryParse(ref new String(wideJsonString.c_str()), &value) || (value == nullptr))
    {
        throw DMException("Failed to parse Json.");
    }

    if (update_state == DEVICE_TWIN_UPDATE_PARTIAL)
    {
        return value;
    }

    if (update_state != DEVICE_TWIN_UPDATE_COMPLETE || value->ValueType != JsonValueType::Object)
    {
        throw DMException("Unknown device twin update type.");
    }

    // Locate the 'desired' node.
    JsonObject^ object = value->GetObject();
    if (object == nullptr)
    {
        throw DMException("Unexpected device twin update element.");
    }

    IJsonValue^ desiredPropertiesNode = nullptr;
    for (IIterator<IKeyValuePair<String^, IJsonValue^>^>^ iter = object->First();
        iter->HasCurrent;
        iter->MoveNext())
    {
        IKeyValuePair<String^, IJsonValue^>^ pair = iter->Current;
        String^ childKey = pair->Key;

        // Look for "desired"
        if (childKey == JsonDesiredNode && pair->Value != nullptr)
        {
            desiredPropertiesNode = pair->Value;
            break;
        }
    }

    if (!desiredPropertiesNode)
    {
        throw DMException("Failed to find the desired properties node.");
    }

    return desiredPropertiesNode;
}

void AzureProxy::OnDesiredProperties(DEVICE_TWIN_UPDATE_STATE update_state, const unsigned char* payload, size_t bufferSize, void* userContextCallback)
{
    TRACE("AzureProxy::OnDesiredProperties()");

    AzureProxy* pThis = static_cast<AzureProxy*>(userContextCallback);
    assert(pThis);

    // Incoming buffer is not null terminated, let's make it into a null-terminated string before parsing.
    string copyOfPayload(reinterpret_cast<const char*>(payload), bufferSize);

    TRACEP("Desired Properties String: ", copyOfPayload.c_str());
    try
    {
        IJsonValue^ desiredValue = GetDesiredPropertiesNode(update_state, copyOfPayload);
        pThis->ProcessDesiredProperties(desiredValue);
    }
    catch (exception&)
    {
        // We just log a message. Let the service continue running.
        TRACE("Error: Failed to process desired properties update.");
    }
}

void AzureProxy::ProcessDesiredProperties(IJsonValue^ desiredPropertyValue)
{
    TRACE(L"ProcessDesiredProperties()");
    switch (desiredPropertyValue->ValueType)
    {
    case JsonValueType::Object:
        {
            // Iterate through the desired properties top-level nodes.
            JsonObject^ object = desiredPropertyValue->GetObject();
            if (object == nullptr)
            {
                TRACE("Warning: Unexpected desired properties contents. Skipping.");
                return;
            }

            for (IIterator<IKeyValuePair<String^, IJsonValue^>^>^ iter = object->First();
                    iter->HasCurrent;
                    iter->MoveNext())
            {
                IKeyValuePair<String^, IJsonValue^>^ pair = iter->Current;
                String^ childKey = pair->Key;

                if (0 == RebootModel::NodeName().compare(childKey->Data()))
                {
                    _rebootModel.SetDesiredProperties(pair->Value);
                }
                else if (0 == AzureUpdateManager::NodeName().compare(childKey->Data()))
                {
                    _azureUpdateManager.SetDesiredProperties(pair->Value);
                }
            }
        }
        break;
    }
}

void AzureProxy::ReportProperties(JsonObject^ root) const
{
    TRACE(L"AzureProxy::ReportProperties()");

    string jsonString = Utils::WideToMultibyte(root->Stringify()->Data());
    TRACEP("Json = ", jsonString.c_str());

    if (IoTHubClient_SendReportedState(_iotHubClientHandle, reinterpret_cast<const unsigned char*>(jsonString.c_str()), jsonString.size() + 1, OnReportedPropertiesSent, NULL) != IOTHUB_CLIENT_OK)
    {
        throw DMException("Failed to send reported properties.");
    }
    TRACE("Reported state has been delivered to IoTHub");
}

void AzureProxy::ReportAllProperties()
{
    TRACE(L"AzureProxy::ReportMonitoredProperties()");

    JsonObject^ root = ref new JsonObject();
    {
        // System Info properties
        root->Insert(ref new String(SystemInfoModel::NodeName().c_str()), _systemInfoModel.GetReportedProperties());

        // Time properties
        root->Insert(ref new String(TimeModel::NodeName().c_str()), _timeModel.GetReportedProperties());

        // Reboot properties
        root->Insert(ref new String(RebootModel::NodeName().c_str()), _rebootModel.GetReportedProperties());

        // Update properties
        root->Insert(ref new String(AzureUpdateManager::NodeName().c_str()), _azureUpdateManager.GetReportedProperties());
    }

    ReportProperties(root);
}
