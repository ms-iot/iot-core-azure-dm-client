#include "stdafx.h"
#include <vector>
#include <string>
#include <assert.h>

#include "AzureAgent.h"
#include "serializer.h"
#include "iothub_client.h"
#include "iothubtransportmqtt.h"
#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/threadapi.h"

#include "Utilities\Logger.h"
#include "Utilities\Utils.h"
#include "LocalAgent\LocalAgent.h"

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

AzureAgent::AzureAgent() :
    _batteryLevel(0),
    _batteryStatus(0),
    _totalMemoryInMB(0),
    _availableMemoryInMB(0)
{
}

bool AzureAgent::Setup(const string& connectionString)
{
    TRACE("AzureAgent::LaunchMonitor()");

    // Prepare the platform
    if (platform_init() != 0)
    {
        TRACE("Error: Failed to initialize the platform.");
        return false;
    }

    // Create an IoTHub client
    _iotHubClientHandle = IoTHubClient_CreateFromConnectionString(connectionString.c_str(), MQTT_Protocol);
    if (_iotHubClientHandle == NULL)
    {
        TRACE("Error: Failure creating IoTHubClient handle");
        return false;
    }

    // Turn on Log 
    bool trace = false;
    IoTHubClient_SetOption(_iotHubClientHandle, "logtrace", &trace);

    // Set the desired properties callback
    if (IoTHubClient_SetDeviceTwinCallback(_iotHubClientHandle, OnDesiredProperties, this) != IOTHUB_CLIENT_OK)
    {
        TRACE("Error: Unable to set device twin callback");
        return false;
    }

    // Set the message properties callback
    if (IoTHubClient_SetMessageCallback(_iotHubClientHandle, OnMessageReceived, this) != IOTHUB_CLIENT_OK)
    {
        TRACE("Error: Unable to set message callback");
        return false;
    }

    return true;
}

void AzureAgent::Shutdown()
{
    TRACE("AzureAgent::StopMonitor()");
    IoTHubClient_Destroy(_iotHubClientHandle);
    platform_deinit();
}

IOTHUBMESSAGE_DISPOSITION_RESULT AzureAgent::OnMessageReceived(IOTHUB_MESSAGE_HANDLE message, void* userContextCallback)
{
    TRACE("AzureAgent::OnMessageReceived()");

    AzureAgent* pThis = static_cast<AzureAgent*>(userContextCallback);
    assert(pThis);

    const char* buffer;
    size_t bufferSize;

    // ToDo: who's responsible for freeing the buffer?
    if (IoTHubMessage_GetByteArray(message, (const unsigned char**)&buffer, &bufferSize) != IOTHUB_MESSAGE_OK)
    {
        TRACE("Error: Unable to retrieve the message data.");
    }
    else
    {
        vector<char> message(bufferSize + 1);
        memcpy(message.data(), buffer, bufferSize);
        message[bufferSize] = '\0';

        TRACEP("Received Message with Data:", message.data());

        pThis->ProcessMessage(message.data());
    }

    return IOTHUBMESSAGE_ACCEPTED;
}

void AzureAgent::OnReportedPropertiesSent(int status_code, void* userContextCallback)
{
    TRACE("AzureAgent::OnReportedPropertiesSent()");

    // AzureAgent* pThis = static_cast<AzureAgent*>(userContextCallback);
    // assert(pThis);

    // ToDo: Log a message to the event log in case of failure code.
    TRACEP("IoTHub: reported properties delivered with status_code :", status_code);
}

#pragma push_macro("GetObject")
#undef GetObject

bool AzureAgent::GetInnerJSon(DEVICE_TWIN_UPDATE_STATE update_state, const string& allJson, IJsonValue^& desiredValue)
{
    TRACE(L"AzureAgent::GetInnerJSon()");
    wstring wideJsonString = Utils::MultibyteToWide(allJson.c_str());

    JsonValue^ value;
    if (!JsonValue::TryParse(ref new String(wideJsonString.c_str()), &value) || (value == nullptr))
    {
        TRACE("Error: Failed to parse Json.");
        return false;
    }

    if (update_state == DEVICE_TWIN_UPDATE_PARTIAL)
    {
        desiredValue = value;
        return true;
    }

    if (update_state != DEVICE_TWIN_UPDATE_COMPLETE)
    {
        return false;
    }

    // Locate the 'desired' node.
    if (value->ValueType != JsonValueType::Object)
    {
        return false;
    }

    JsonObject^ object = value->GetObject();
    if (object == nullptr)
    {
        return false;
    }

    for (IIterator<IKeyValuePair<String^, IJsonValue^>^>^ iter = object->First();
        iter->HasCurrent;
        iter->MoveNext())
    {
        IKeyValuePair<String^, IJsonValue^>^ pair = iter->Current;
        String^ childKey = pair->Key;

        // Look for "desired"
        if (0 == wcscmp(childKey->Data(), JsonDesiredNode) && (pair->Value != nullptr))
        {
            desiredValue = pair->Value;
            return true;
        }
    }
    return false;
}

void AzureAgent::OnDesiredProperties(DEVICE_TWIN_UPDATE_STATE update_state, const unsigned char* payLoad, size_t bufferSize, void* userContextCallback)
{
    TRACE("AzureAgent::OnDesiredProperties()");

    AzureAgent* pThis = static_cast<AzureAgent*>(userContextCallback);
    assert(pThis);

    // Incoming buffer is not null terminated, let's make it into a null-terminated string before parsing.
    string copyOfPayload(reinterpret_cast<const char*>(payLoad), bufferSize);

    TRACEP("Desired Propertie String: ", copyOfPayload.c_str());

    IJsonValue^ desiredValue;
    if (GetInnerJSon(update_state, copyOfPayload, desiredValue))
    {
        pThis->ProcessDesiredProperties(desiredValue);
    }
}

bool AzureAgent::ProcessDesiredProperties(IJsonValue^ desiredPropertyValue)
{
    TRACE(L"ProcessDesiredProperties()");
    switch (desiredPropertyValue->ValueType)
    {
    case JsonValueType::Object:
    {
        // Iterate through the desired properties top-level nodes.
        JsonObject^ object = desiredPropertyValue->GetObject();
        if (object != nullptr)
        {
            // auto iter = object->First();
            for (IIterator<IKeyValuePair<String^, IJsonValue^>^>^ iter = object->First();
                 iter->HasCurrent;
                 iter->MoveNext())
            {
                IKeyValuePair<String^, IJsonValue^>^ pair = iter->Current;
                String^ childKey = pair->Key;
                if (0 == wcscmp(childKey->Data(), JsonRebootW))
                {
                    OnReboot(pair->Value);
                }
            }
        }
    }
    break;
    }
    return true;
}

#pragma pop_macro("GetObject")

bool AzureAgent::ProcessMessage(const string& command)
{
    TRACEP("AzureAgent::ProcessMessage() : ", command.c_str());
    if (command == JsonReboot)
    {
        wprintf(L"Invoking local agent reboot!\n");
        LocalAgent::Reboot();
    }
    return true;
}

// Sample code for desired properties.
bool AzureAgent::OnReboot(IJsonValue^ rebootNode)
{
    TRACE(L"OnReboot()");
    JsonValueType type = rebootNode->ValueType;
    if (type == JsonValueType::String)
    {
        String^ childValueString = rebootNode->GetString();
        TRACE(L"OnReboot() should not be called through the 'desired' properties.");
        // LocalAgent::Reboot();
    }
    return true;
}

void AzureAgent::SetBatteryLevel(unsigned int level)
{
    TRACEP(L"SetBatteryLevel() :", level);
    _batteryLevel = level;
}

void AzureAgent::SetBatteryStatus(unsigned int status)
{
    TRACEP(L"SetBatteryStatus() :", status);
    _batteryStatus = status;
}

void AzureAgent::SetTotalMemoryMB(unsigned int memoryInMBs)
{
    TRACEP(L"SetTotalMemory() :", memoryInMBs);
    _totalMemoryInMB = memoryInMBs;
}

void AzureAgent::SetAvailableMemoryMB(unsigned int memoryInMBs)
{
    TRACEP(L"SetAvailableMemory() :", memoryInMBs);
    _availableMemoryInMB = memoryInMBs;
}

bool AzureAgent::ReportProperties()
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
        TRACE("Error: Failure sending data");
        return false;
    }
    TRACE("Reported state has been delivered to IoTHub");
    return true;
}
