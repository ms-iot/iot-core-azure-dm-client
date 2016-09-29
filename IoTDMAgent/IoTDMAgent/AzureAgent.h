#pragma once

#include <string>
#include "iothub_client.h"
#include "parson.h"

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
    static void OnDesiredProperties(DEVICE_TWIN_UPDATE_STATE update_state, const unsigned char* payLoad, size_t size, void* userContextCallback);
    static IOTHUBMESSAGE_DISPOSITION_RESULT OnMessageReceived(IOTHUB_MESSAGE_HANDLE message, void* userContextCallback);
    static bool GetInnerJSon(DEVICE_TWIN_UPDATE_STATE update_state, JSON_Value* allJSON, bool& partial, std::string& innerJSon);

    bool ProcessMessage(const std::string& command);
    bool ProcessDesiredProperties(const std::string& jsonString);
    bool OnReboot(Windows::Data::Json::IJsonValue^ rebootNode);

    // Data members
    IOTHUB_CLIENT_HANDLE _iotHubClientHandle;

    // Reported Properties
    unsigned int _batteryLevel;
    unsigned int _batteryStatus;
    unsigned int _totalMemoryInMB;
    unsigned int _availableMemoryInMB;
};


