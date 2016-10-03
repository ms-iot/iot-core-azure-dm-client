#pragma once

#include <string>
#include "iothub_client.h"

class AzureAgent
{
public:
    AzureAgent();

    bool Setup(const std::string& connectionString);
    void Shutdown();

    void SetBatteryLevel(unsigned int level);
    void SetBatteryStatus(unsigned int status);
    void SetTotalMemoryMB(unsigned int memoryInMBs);
    void SetAvailableMemoryMB(unsigned int memoryInMBs);

    bool ReportProperties();

private:
    static void OnReportedPropertiesSent(int status_code, void* userContextCallback);
    static void OnDesiredProperties(DEVICE_TWIN_UPDATE_STATE update_state, const unsigned char* payload, size_t size, void* userContextCallback);
    static IOTHUBMESSAGE_DISPOSITION_RESULT OnMessageReceived(IOTHUB_MESSAGE_HANDLE message, void* userContextCallback);
    static bool GetInnerJSon(DEVICE_TWIN_UPDATE_STATE update_state, const std::string& allJson, Windows::Data::Json::IJsonValue^& desiredValue);

    bool ProcessMessage(const std::string& command);
    bool ProcessDesiredProperties(Windows::Data::Json::IJsonValue^ value);

    // Sample code for desired properties.
    bool OnReboot(Windows::Data::Json::IJsonValue^ rebootNode);
    bool OnProp1(Windows::Data::Json::IJsonValue^ prop1Node);

    // Data members
    IOTHUB_CLIENT_HANDLE _iotHubClientHandle;

    // Reported Properties
    unsigned int _batteryLevel;
    unsigned int _batteryStatus;
    unsigned int _totalMemoryInMB;
    unsigned int _availableMemoryInMB;
};


