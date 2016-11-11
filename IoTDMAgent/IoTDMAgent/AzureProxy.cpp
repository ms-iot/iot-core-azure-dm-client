#include "stdafx.h"

#include "AzureProxy.h"
#include "iothub_client.h"
#include "iothubtransportmqtt.h"
#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/threadapi.h"

#include "LocalMachine\LocalMachine.h"
#include "LocalMachine\CSPs\RebootCSP.h"

#include "AzureModels\ModelManager.h"

using namespace Windows::Data::Json;
using namespace Windows::Foundation::Collections;
using namespace Platform;
using namespace std;

const char* ReportMethod = "Report";
const char* RebootMethod = "Reboot";
const char* RemoteWipeMethod = "RemoteWipe";

AzureProxy::AzureProxy(TaskQueue* taskQueue) :
    _taskQueue(taskQueue),
    _iotHubClientHandle(nullptr)
{
    TRACE(__FUNCTION__);
}

void AzureProxy::Connect(const std::string& connectionString)
{
    TRACE(__FUNCTION__);

    Disconnect();

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

void AzureProxy::Disconnect()
{
    TRACE(__FUNCTION__);
    if (_iotHubClientHandle != nullptr)
    {
        IoTHubClient_Destroy(_iotHubClientHandle);
        platform_deinit();
        _iotHubClientHandle = NULL;
    }
}

AzureProxy::~AzureProxy()
{
    TRACE(__FUNCTION__);
    Disconnect();
}

int AzureProxy::OnMethodCalled(const char* method_name, const unsigned char* payload, size_t size, unsigned char** response, size_t* resp_size, void* userContextCallback)
{
    TRACE(__FUNCTION__);

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

void AzureProxy::ProcessReportAllCall()
{
    TRACE(__FUNCTION__);

    TaskQueue::Task task([this](ModelManager* modelManager)
    {
        string allProperties = modelManager->GetAllPropertiesJson();
        ReportProperties(allProperties);
        return "";
    });

    _taskQueue->Enqueue(move(task));
}

string AzureProxy::ProcessRebootCall()
{
    TRACE(__FUNCTION__);

    TaskQueue::Task task([](ModelManager* modelManager)
    {
        return modelManager->ExecRebootNow();
    });

    future<string> futureResult = _taskQueue->Enqueue(move(task));

    string response = futureResult.get();

    // There is a bug in the Azure Client SDK that prevents us from sending reported properties
    // on a different thread while we are in a method callback.
    // To work around that, we are splitting the sending into a separate work item which
    // we do not wait for (and will get unblocked when the method callback returns).
    // taskItem = make_shared<TaskItem>();
    TaskQueue::Task responseTask([this](ModelManager* modelManager)
    {
        string allProperties = modelManager->GetRebootPropertiesJson();
        ReportProperties(allProperties);
        return "";
    });

    _taskQueue->Enqueue(move(responseTask));

    return response;
}

string AzureProxy::ProcessRemoteWipe()
{
    TRACE(__FUNCTION__);

    TaskQueue::Task task([](ModelManager* modelManager)
    {
        return modelManager->ExecWipeNow();
    });

    future<string> futureResult = _taskQueue->Enqueue(move(task));

    string response = futureResult.get();

    // There is a bug in the Azure Client SDK that prevents us from sending reported properties
    // on a different thread while we are in a method callback.
    // To work around that, we are splitting the sending into a separate work item which
    // we do not wait for (and will get unblocked when the method callback returns).
    // taskItem = make_shared<TaskItem>();

    TaskQueue::Task responseTask([this](ModelManager* modelManager)
    {
        string allProperties = modelManager->GetWipePropertiesJson();
        ReportProperties(allProperties);
        return "";
    });

    _taskQueue->Enqueue(move(responseTask));

    return response;
}

int AzureProxy::ProcessMethodCall(const string& name, const string& payload, string& response)
{
    TRACE("AzureProxy::ProcessMethodCall()");

    int result = IOTHUB_CLIENT_IOTHUB_METHOD_STATUS_SUCCESS;

    try
    {
        if (name == ReportMethod)
        {
            ProcessReportAllCall();
        }
        else if (name == RebootMethod)
        {
            response = ProcessRebootCall();
        }
        else if (name == RemoteWipeMethod)
        {
            response = ProcessRemoteWipe();
        }
    }
    catch (const exception& e)
    {
        TRACEP("AzureProxy::ProcessMethodCall() failed: ", e.what());
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

void AzureProxy::ReportProperties(std::string allJson) const
{
    TRACE(__FUNCTION__);

    if (IoTHubClient_SendReportedState(_iotHubClientHandle, reinterpret_cast<const unsigned char*>(allJson.c_str()), allJson.size() + 1, OnReportedPropertiesSent, NULL) != IOTHUB_CLIENT_OK)
    {
        throw DMException("Failed to send reported properties.");
    }
    TRACE("Reported state has been delivered to IoTHub");
}

void AzureProxy::OnDesiredProperties(DEVICE_TWIN_UPDATE_STATE update_state, const unsigned char* payload, size_t bufferSize, void* userContextCallback)
{
    TRACE(__FUNCTION__);

    AzureProxy* pThis = static_cast<AzureProxy*>(userContextCallback);
    assert(pThis);

    // Incoming buffer is not null terminated, let's make it into a null-terminated string before parsing.
    string copyOfPayload(reinterpret_cast<const char*>(payload), bufferSize);

    TRACEP("Desired Properties String: ", copyOfPayload.c_str());
    try
    {
        TaskQueue::Task task([pThis, update_state, copyOfPayload](ModelManager* modelManager)
        {
            modelManager->ProcessDesiredProperties(update_state == DEVICE_TWIN_UPDATE_COMPLETE, copyOfPayload);
            return "";  // A return payload does not apply to property changes.
                        // However, to comply with the task type, we have to return a string.
        });

        // Note that Enqueue() may throw if enqueuing is disabled.
        pThis->_taskQueue->Enqueue(move(task));

        // We do not need to inspect the result in this function - so, no need to do futureResult.get() etc.
    }
    catch (const exception&)
    {
        // We just log a message. Let the service continue running.
        TRACE("Error: Failed to process desired properties update.");
    }
}
