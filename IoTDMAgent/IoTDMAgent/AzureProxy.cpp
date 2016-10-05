#include "stdafx.h"
#include <vector>
#include <string>
#include <assert.h>

#include "AzureProxy.h"
#include "serializer.h"
#include "iothub_client.h"
#include "iothubtransportmqtt.h"
#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/threadapi.h"

#include "Utilities\Logger.h"
#include "Utilities\Utils.h"
#include "Utilities\DMException.h"
#include "LocalMachine\LocalMachine.h"

using namespace Windows::Data::Json;
using namespace Windows::Foundation::Collections;
using namespace Platform;
using namespace std;

#define JsonMemory "Memory"
#define JsonTotalMemory "TotalMemory"
#define JsonAvailableMemory "AvailableMemory"

#define JsonBattery "Battery"
#define JsonBatteryLevel "Level"
#define JsonBatteryStatus "Status"

#define JsonReboot "Reboot"
#define JsonRebootW L"Reboot"

#define JsonDesiredNode L"desired"

#define JsonDesiredProp1 L"Prop1"

AzureProxy::AzureProxy(const string& connectionString) :
    _iotHubClientHandle(nullptr),
    _batteryLevel(0),
    _batteryStatus(0),
    _totalMemoryInMB(0),
    _availableMemoryInMB(0)
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
        pThis->ProcessMessage(message);
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

    TRACEP("Desired Propertie String: ", copyOfPayload.c_str());
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
                if (childKey == JsonRebootW)
                {
                    OnReboot(pair->Value);
                }
            }
        }
        break;
    }
}

void AzureProxy::ProcessMessage(const string& command)
{
    TRACEP("AzureProxy::ProcessMessage() : ", command.c_str());
    if (command == JsonReboot)
    {
        wprintf(L"Invoking local agent reboot!\n");
        LocalMachine::Reboot();
    }
}

// Sample code for desired properties.
void AzureProxy::OnReboot(IJsonValue^ rebootNode)
{
    TRACE(L"OnReboot()");
    JsonValueType type = rebootNode->ValueType;
    if (type == JsonValueType::String)
    {
        String^ childValueString = rebootNode->GetString();
        TRACE(L"OnReboot() should not be called through the 'desired' properties.");
        // ToDo: implement the parsing of the Reboot node.
        // LocalMachine::Reboot();
    }
}

void AzureProxy::SetBatteryLevel(unsigned int level)
{
    TRACEP(L"SetBatteryLevel() :", level);
    _batteryLevel = level;
}

void AzureProxy::SetBatteryStatus(unsigned int status)
{
    TRACEP(L"SetBatteryStatus() :", status);
    _batteryStatus = status;
}

void AzureProxy::SetTotalMemoryMB(unsigned int memoryInMBs)
{
    TRACEP(L"SetTotalMemory() :", memoryInMBs);
    _totalMemoryInMB = memoryInMBs;
}

void AzureProxy::SetAvailableMemoryMB(unsigned int memoryInMBs)
{
    TRACEP(L"SetAvailableMemory() :", memoryInMBs);
    _availableMemoryInMB = memoryInMBs;
}

void AzureProxy::ReportProperties()
{
    TRACE(L"ReportProperties()");

    JsonObject^ memoryProperties = ref new JsonObject();
    memoryProperties->Insert(JsonTotalMemory, JsonValue::CreateNumberValue(_totalMemoryInMB));
    memoryProperties->Insert(JsonAvailableMemory, JsonValue::CreateNumberValue(_availableMemoryInMB));

    JsonObject^ batteryProperties = ref new JsonObject();
    batteryProperties->Insert(JsonBatteryLevel, JsonValue::CreateNumberValue(_batteryLevel));
    batteryProperties->Insert(JsonBatteryStatus, JsonValue::CreateNumberValue(_batteryStatus));

    JsonObject^ root = ref new JsonObject();
    root->Insert(JsonMemory, memoryProperties);
    root->Insert(JsonBattery, batteryProperties);

    string jsonString = Utils::WideToMultibyte(root->Stringify()->Data());
    TRACEP("Json = ", jsonString.c_str());

    // ToDo: const char* -> unsigned char*; why not use simple conversion?
    vector<unsigned char> v(jsonString.size() + 1);
    memcpy(v.data(), jsonString.c_str(), jsonString.size() + 1);

    // Sending the serialized reported properties to IoTHub
    if (IoTHubClient_SendReportedState(_iotHubClientHandle, v.data(), v.size(), OnReportedPropertiesSent, NULL) != IOTHUB_CLIENT_OK)
    {
        throw DMException("Failed to send reported properties.");
    }
    TRACE("Reported state has been delivered to IoTHub");
}
