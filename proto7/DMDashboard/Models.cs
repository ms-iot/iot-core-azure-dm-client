using System;

namespace Microsoft.Devices.Management
{
    public class RebootInfo
    {
        public DateTime lastRebootTime;
        public DateTime lastRebootCmdTime;
        public DateTime singleRebootTime;
        public DateTime dailyRebootTime;
    }

    public class TimeZoneInfo
    {
        public long bias;
        public string standardName;
        public DateTime standardDate;
        public long standardBias;
        public string daylightName;
        public DateTime daylightDate;
        public long daylightBias;
    }

    public class TimeInfo
    {
        public TimeInfo()
        {
            timeZone = new TimeZoneInfo();
        }

        public DateTime localTime;
        public string ntpServer;
        public TimeZoneInfo timeZone;
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


    //public const string ReportedTimeInfoProperty = "properties.reported.timeInfo";
    //public const string DesiredTimeInfoProperty = "properties.desired.timeInfo";

}