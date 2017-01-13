#pragma once

#include <string>
#include "TaskQueue.h"
#include "iothub_client.h"

class AzureProxy
{
public:
    AzureProxy(TaskQueue* taskQueue);
    ~AzureProxy();

    void Connect(const std::string& connectionString);
    void Disconnect();

private:

    // Azure callbacks
    static void OnReportedPropertiesSent(int status_code, void* userContextCallback);
    static void OnDesiredProperties(DEVICE_TWIN_UPDATE_STATE update_state, const unsigned char* payload, size_t size, void* userContextCallback);
    static IOTHUBMESSAGE_DISPOSITION_RESULT OnMessageReceived(IOTHUB_MESSAGE_HANDLE message, void* userContextCallback);
    static int OnMethodCalled(const char* method_name, const unsigned char* payload, size_t size, unsigned char** response, size_t* resp_size, void* userContextCallback);

    // Helpers
    void ProcessMessage(const std::string& command);
    int ProcessMethodCall(const std::string& name, const std::string& payload, std::string& response);
    void ProcessReportAllCall();
    std::string ProcessRebootCall();
    std::string ProcessRemoteWipe();

    void ReportProperties(std::string allJson) const;

    // Data members
    IOTHUB_CLIENT_HANDLE _iotHubClientHandle;
    TaskQueue* _taskQueue;
};


