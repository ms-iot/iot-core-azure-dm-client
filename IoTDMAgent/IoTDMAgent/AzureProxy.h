#pragma once

#include <string>
#include "iothub_client.h"

class AzureAgent
{
public:
    AzureAgent();

    void Setup(const std::string& connectionString);
    void Shutdown();

    void SetBatteryLevel(unsigned int level);
    void SetBatteryStatus(unsigned int status);
    void SetTotalMemoryMB(unsigned int memoryInMBs);
    void SetAvailableMemoryMB(unsigned int memoryInMBs);

    void ReportProperties() noexcept;

private:
    static void OnReportedPropertiesSent(int status_code, void* userContextCallback);
    static void OnDesiredProperties(DEVICE_TWIN_UPDATE_STATE update_state, const unsigned char* payload, size_t size, void* userContextCallback);
    static IOTHUBMESSAGE_DISPOSITION_RESULT OnMessageReceived(IOTHUB_MESSAGE_HANDLE message, void* userContextCallback);
    static Windows::Data::Json::IJsonValue^ GetInnerJSon(DEVICE_TWIN_UPDATE_STATE update_state, const std::string& allJson);

    void ProcessMessage(const std::string& command);
    void ProcessDesiredProperties(Windows::Data::Json::IJsonValue^ value);

    // Sample code for desired properties.
    void OnReboot(Windows::Data::Json::IJsonValue^ rebootNode);
    void OnProp1(Windows::Data::Json::IJsonValue^ prop1Node);

    // Data members
    IOTHUB_CLIENT_HANDLE _iotHubClientHandle;

    // Reported Properties
    unsigned int _batteryLevel;
    unsigned int _batteryStatus;
    unsigned int _totalMemoryInMB;
    unsigned int _availableMemoryInMB;
};


