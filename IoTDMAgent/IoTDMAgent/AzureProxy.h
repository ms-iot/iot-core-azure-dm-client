#pragma once

#include <string>
#include "TaskQueue.h"
#include "iothub_client.h"
#include "AzureModels\RebootModel.h"
#include "AzureModels\RemoteWipeModel.h"
#include "AzureModels\TimeModel.h"
#include "AzureModels\SystemInfoModel.h"
#include "AzureModels\AzureUpdateManager.h"

class AzureProxy
{
public:
    AzureProxy(TaskQueue* taskQueue);
    ~AzureProxy();

    void Connect(const std::string& connectionString);
    void Disconnect();

    void ReportRebootProperties();
    void ReportRemoteWipeProperties();
    void ReportAllProperties();

    void ProcessDesiredProperties(bool completeSet, const std::string& allJson);

private:

    static void OnReportedPropertiesSent(int status_code, void* userContextCallback);
    static void OnDesiredProperties(DEVICE_TWIN_UPDATE_STATE update_state, const unsigned char* payload, size_t size, void* userContextCallback);
    static IOTHUBMESSAGE_DISPOSITION_RESULT OnMessageReceived(IOTHUB_MESSAGE_HANDLE message, void* userContextCallback);
    static int OnMethodCalled(const char* method_name, const unsigned char* payload, size_t size, unsigned char** response, size_t* resp_size, void* userContextCallback);
    static Windows::Data::Json::IJsonValue^ GetDesiredPropertiesNode(DEVICE_TWIN_UPDATE_STATE update_state, const std::string& allJson);

    void ProcessMessage(const std::string& command);
    int ProcessMethodCall(const std::string& name, const std::string& payload, std::string& response);
    void ProcessReportAllCall();
    std::string ProcessRebootCall();
    std::string ProcessRemoteWipe();
    void ProcessDesiredProperties(Windows::Data::Json::IJsonValue^ value);

    void ReportProperties(Windows::Data::Json::JsonObject^ root) const;

    // Data members
    IOTHUB_CLIENT_HANDLE _iotHubClientHandle;

    TaskQueue* _taskQueue;

    SystemInfoModel _systemInfoModel;
    TimeModel _timeModel;
    RebootModel _rebootModel;
    RemoteWipeModel _remoteWipeModel;
    AzureUpdateManager _azureUpdateManager;
};


