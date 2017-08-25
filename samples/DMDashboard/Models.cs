/*
Copyright 2017 Microsoft
Permission is hereby granted, free of charge, to any person obtaining a copy of this software 
and associated documentation files (the "Software"), to deal in the Software without restriction, 
including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, 
subject to the following conditions:

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT 
LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH 
THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
using System;
using Newtonsoft.Json;

namespace Microsoft.Devices.Management
{
    namespace WindowsUpdates
    {
        public class GetResponse
        {
            public string installed;
            public string approved;
            public string failed;
            public string installable;
            public string pendingReboot;
            public string lastScanTime;
            public bool deferUpgrade;
        }

        public class SetParams
        {
            public string SectionName
            {
                get
                {
                    return "windowsUpdates";
                }
            }

            public string approved;

            public string ToJson()
            {
                return "\"windowsUpdates\" : " + JsonConvert.SerializeObject(this);
            }
        }
    }

    class Certificates
    {
        public class CertificateConfiguration
        {
            public string SectionName
            {
                get
                {
                    return "certificates";
                }
            }

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

        public CertificateConfiguration Configuration = null;
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
        public int Tag = 0;
        public int Status = 0;
        public string ValidTo = "";
        public string ValidFrom = "";
        public string IssuedTo = "";
        public string IssuedBy = "";
    }
    class GetWifiProfileDetailsParams
    {
        public string profileName;
        public string connectionString;
        public string containerName;
        public string blobName;
    }
}
