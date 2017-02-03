using System;

namespace Microsoft.Devices.Management
{
    // ToDo: Need to move this to a common place for both DMDashboard and IoTDMClientLib.
    public class RebootInfo
    {
        public DateTime lastRebootTime;
        public DateTime lastRebootCmdTime;
        public DateTime singleRebootTime;
        public DateTime dailyRebootTime;
    }

    public class TimeInfo
    {
        public int Tag;
        public int Status;
        public long TimeZoneDaylightBias;
        public DateTime TimeZoneDaylightDate;
        public string TimeZoneDaylightName;
        public long TimeZoneStandardBias;
        public DateTime TimeZoneStandardDate;
        public string TimeZoneStandardName;
        public long TimeZoneBias;
        public DateTime LocalTime;
        public string NtpServer;
    }

    public struct DeviceStatus
    {
        public long secureBootState;
        public string macAddressIpV4;
        public string macAddressIpV6;
        public bool macAddressIsConnected;
        public long macAddressType;
        public string osType;
        public long batteryStatus;
        public long batteryRemaining;
        public long batteryRuntime;
    }

    class DesiredProperties
    {
        public TimeInfo timeInfo;
        public RebootInfo rebootInfo;
    }

    class PropertiesRoot
    {
        public DesiredProperties desired;
    }
}