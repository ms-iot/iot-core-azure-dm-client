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

    namespace TimeInfo
    {
        public class GetResponse
        {
            public long TimeZoneDaylightBias;
            public string TimeZoneDaylightDate;
            public string TimeZoneDaylightName;
            public long TimeZoneStandardBias;
            public string TimeZoneStandardDate;
            public string TimeZoneStandardName;
            public long TimeZoneBias;
            public string LocalTime;
            public string NtpServer;
        }

        public class SetParams
        {
            public long TimeZoneDaylightBias;
            public string TimeZoneDaylightDate;
            public string TimeZoneDaylightName;
            public long TimeZoneStandardBias;
            public string TimeZoneStandardDate;
            public string TimeZoneStandardName;
            public long TimeZoneBias;
            public string NtpServer;
        }
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

    class DesiredManagementProperties
    {
        public TimeInfo.SetParams timeInfo;
        public RebootInfo rebootInfo;
    }

    class DesiredMicrosoftProperties
    {
        public DesiredManagementProperties management;

        public DesiredMicrosoftProperties()
        {
            management = new DesiredManagementProperties();
        }
    }

    class DesiredProperties
    {
        public DesiredMicrosoftProperties microsoft;

        public DesiredProperties()
        {
            microsoft = new DesiredMicrosoftProperties();
        }
    }

    class PropertiesRoot
    {
        public DesiredProperties desired;

        public PropertiesRoot()
        {
            desired = new DesiredProperties();
        }
    }
}