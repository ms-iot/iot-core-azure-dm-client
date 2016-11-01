#pragma once

#include <string>
#include "iothub_client.h"
#include "AzureModels\RebootModel.h"
#include "AzureModels\RemoteWipeModel.h"
#include "AzureModels\TimeModel.h"
#include "AzureModels\SystemInfoModel.h"
#include "AzureModels\AzureUpdateManager.h"

class AzureProxy
{
public:
    AzureProxy(const std::string& connectionString);
    ~AzureProxy();

    void ReportAllProperties();

private:
    static void OnReportedPropertiesSent(int status_code, void* userContextCallback);
    static void OnDesiredProperties(DEVICE_TWIN_UPDATE_STATE update_state, const unsigned char* payload, size_t size, void* userContextCallback);
    static IOTHUBMESSAGE_DISPOSITION_RESULT OnMessageReceived(IOTHUB_MESSAGE_HANDLE message, void* userContextCallback);
    static int OnMethodCalled(const char* method_name, const unsigned char* payload, size_t size, unsigned char** response, size_t* resp_size, void* userContextCallback);
    static Windows::Data::Json::IJsonValue^ GetDesiredPropertiesNode(DEVICE_TWIN_UPDATE_STATE update_state, const std::string& allJson);

    void ProcessMessage(const std::string& command);
    int ProcessMethodCall(const std::string& name, const std::string& payload, std::string& response);
    void ProcessDesiredProperties(Windows::Data::Json::IJsonValue^ value);

    void ReportProperties(Windows::Data::Json::JsonObject^ root) const;

    // Data members
    IOTHUB_CLIENT_HANDLE _iotHubClientHandle;

    SystemInfoModel _systemInfoModel;
    TimeModel _timeModel;
    RebootModel _rebootModel;
    RemoteWipeModel _remoteWipeModel;
    AzureUpdateManager _azureUpdateManager;
};


