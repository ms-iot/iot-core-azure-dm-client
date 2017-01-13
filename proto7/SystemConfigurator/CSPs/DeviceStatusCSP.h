#pragma once

#include <string>

class DeviceStatusCSP
{
    struct DeviceStatusInfo
    {
        long secureBootState;
        std::wstring macAddressIpV4;
        std::wstring macAddressIpV6;
        bool macAddressIsConnected;
        long macAddressType;
        std::wstring osType;
        long batteryStatus;
        long batteryRemaining;
        long batteryRuntime;
    };

public:
    static std::wstring GetDeviceStatusJson();

private:
    static void GetDeviceStatus(DeviceStatusInfo& deviceStatusInfo);
};