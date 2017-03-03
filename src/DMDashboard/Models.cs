using System;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;

namespace Microsoft.Devices.Management
{
    namespace RebootInfo
    {
        public class GetResponse
        {
            public string lastBootTime;
            public string lastRebootCmdTime;
            public string lastRebootCmdStatus;
            public string singleRebootTime;
            public string dailyRebootTime;
        }

        public class SetParams
        {
            public DateTime singleRebootTime;
            public DateTime dailyRebootTime;

            public string ToJson()
            {
                return "\"scheduledReboot\" : " + JsonConvert.SerializeObject(this);
            }
        }
    }

    namespace TimeInfo
    {
        public class GetResponse
        {
            public long timeZoneDaylightBias;
            public string timeZoneDaylightDate;
            public string timeZoneDaylightName;
            public long timeZoneStandardBias;
            public string timeZoneStandardDate;
            public string timeZoneStandardName;
            public long timeZoneBias;
            public string localTime;
            public string ntpServer;
        }

        public class SetParams
        {
            public long timeZoneDaylightBias;
            public string timeZoneDaylightDate;
            public string timeZoneDaylightName;
            public long timeZoneStandardBias;
            public string timeZoneStandardDate;
            public string timeZoneStandardName;
            public long timeZoneBias;
            public string ntpServer;

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

    class GetCertificateDetailsParams
    {
        public string path;
        public string hash;
        public string connectionString;
        public string containerName;
        public string blobName;
    }

    class GetCertificateDetailsResponse
    {
        public int Tag;
        public int Status;
        public string ValidTo;
        public string ValidFrom;
        public string IssuedTo;
        public string IssuedBy;
    }
}