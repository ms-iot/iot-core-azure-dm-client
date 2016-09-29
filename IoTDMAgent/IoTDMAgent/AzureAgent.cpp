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
#include "parson.h"

#include "..\Utilities\Logger.h"
#include "..\Utilities\Utils.h"
#include "..\LocalAgent\LocalAgent.h"

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

    TRACEP("IoTHub: reported properties delivered with status_code :", status_code);
}

bool AzureAgent::GetInnerJSon(DEVICE_TWIN_UPDATE_STATE update_state, JSON_Value* allJSON, bool& partial, string& innerJSon)
{
    JSON_Object *allObject = json_value_get_object(allJSON);
    if (allObject == NULL)
    {
        TRACE("Error: Failure in json_value_get_object");
        return false;
    }

    switch (update_state)
    {
        case DEVICE_TWIN_UPDATE_COMPLETE:
        {
            TRACE("Complete");

            JSON_Object* desired = json_object_get_object(allObject, "desired");
            if (desired == NULL)
            {
                TRACE("Error: failure in json_object_get_object");
            }
            else
            {
                json_object_remove(desired, "$version"); //it might not exist
                JSON_Value* desiredAfterRemove = json_object_get_value(allObject, "desired");
                if (desiredAfterRemove != NULL)
                {
                    char* pretty = json_serialize_to_string(desiredAfterRemove);
                    if (pretty == NULL)
                    {
                        TRACE("Error: failure in json_serialize_to_string");
                    }
                    else
                    {
                        TRACEP("Received inner json (complete):\n", pretty);
                        partial = false;
                        innerJSon = pretty;
                        free(pretty);
                    }
                }
            }
            break;
        }
        case DEVICE_TWIN_UPDATE_PARTIAL:
        {
            TRACE("Partial");

            json_object_remove(allObject, "$version");
            char* pretty = json_serialize_to_string(allJSON);
            if (pretty == NULL)
            {
                TRACE("Error: failure in json_serialize_to_string");
            }
            else
            {
                TRACEP("Received inner json (partial):\n", pretty);
                partial = true;
                innerJSon = pretty;
                free(pretty);
            }
            break;
        }
        default:
        {
            TRACEP("Error: IotHub SDK Internal Error: unexpected value for update_state =", (int)update_state);
        }
    }

    return true;
}

void AzureAgent::OnDesiredProperties(DEVICE_TWIN_UPDATE_STATE update_state, const unsigned char* payLoad, size_t bufferSize, void* userContextCallback)
{
    TRACE("AzureAgent::OnDesiredProperties()");

    AzureAgent* pThis = static_cast<AzureAgent*>(userContextCallback);
    assert(pThis);

    vector<char> copyOfPayload(bufferSize + 1);
    memcpy(copyOfPayload.data(), payLoad, bufferSize);
    copyOfPayload[bufferSize] = '\0';

    JSON_Value* allJSON = json_parse_string(copyOfPayload.data());
    if (allJSON == NULL)
    {
        TRACE("Error: Failure in json_parse_string");
        return;
    }

    bool partial = true;
    string innerJSon;
    if (GetInnerJSon(update_state, allJSON, partial, innerJSon))
    {
        pThis->ProcessDesiredProperties(innerJSon);
    }

    json_value_free(allJSON);
}

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

#pragma push_macro("GetObject")
#undef GetObject

bool AzureAgent::ProcessDesiredProperties(const string& jsonString)
{
    TRACE(L"ProcessDesiredProperties()");
    wstring wideJsonString = Utils::MultibyteToWide(jsonString.c_str());

    JsonValue^ value;
    if (!JsonValue::TryParse(ref new String(wideJsonString.c_str()), &value) || (value == nullptr))
    {
        TRACE("Error: Failed to parse Json.");
        return false;
    }

    switch (value->ValueType)
    {
    case JsonValueType::Object:
    {
        // Iterate through the desired properties top-level nodes.
        JsonObject^ object = value->GetObject();
        if (object != nullptr)
        {
            // auto iter = object->First();
            for (IIterator<IKeyValuePair<String^, IJsonValue^>^>^ iter = object->First();
                 iter->HasCurrent;
                 iter->MoveNext())
            {
#if 0
                // Sample code (note that reboot is implemented as a message right now).
                IKeyValuePair<String^, IJsonValue^>^ pair = iter->Current;
                String^ childKey = pair->Key;
                if (0 == wcscmp(childKey->Data(), JsonReboot))
                {
                    OnReboot(pair->Value);
                }
#endif
            }
        }
    }
    break;
    }
    return true;
}

#pragma pop_macro("GetObject")

#if 0
// Sample code for desired properties.
bool AzureAgent::OnReboot(IJsonValue^ rebootNode)
{
    TRACE(L"OnReboot()");
    JsonValueType type = rebootNode->ValueType;
    if (type == JsonValueType::String)
    {
        String^ childValueString = rebootNode->GetString();
        TRACE(L"OnReboot() should not be called through the 'desired' properties.");
        LocalAgent::Reboot();
    }
    return true;
}
#endif

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
