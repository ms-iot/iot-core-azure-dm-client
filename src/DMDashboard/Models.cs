using System;
using Newtonsoft.Json;

namespace Microsoft.Devices.Management
{
    // ToDo: Need to move this to a common place for both DMDashboard and IoTDMClientLib.
    public class RebootInfo
    {
        public DateTime lastRebootTime;
        public DateTime lastRebootCmdTime;
        public DateTime singleRebootTime;
        public DateTime dailyRebootTime;

        public string ToJson()
        {
            return "\"rebootInfo\" : " + JsonConvert.SerializeObject(this);
        }
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

            public string ToJson()
            {
                return "\"timeInfo\" : " + JsonConvert.SerializeObject(this);
            }
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

    class ExternalStorage
    {
        public string connectionString;
        public string container;

        public string ToJson()
        {
            return "\"externalStorage\" : " + JsonConvert.SerializeObject(this);
        }
    }

    class Certificates
    {
        public class CertificateConfiguration
        {
            public string rootCATrustedCertificates_Root;
            public string rootCATrustedCertificates_CA;
            public string rootCATrustedCertificates_TrustedPublisher;
            public string rootCATrustedCertificates_TrustedPeople;
            public string certificateStore_CA_System;
            public string certificateStore_Root_System;
            public string certificateStore_My_User;
            public string certificateStore_My_System;

            public string ToJson()
            {
                return "\"certificates\" : " + JsonConvert.SerializeObject(this);
            }
        }

        public uint Tag;
        public uint Status;
        public CertificateConfiguration Configuration;
    }
}