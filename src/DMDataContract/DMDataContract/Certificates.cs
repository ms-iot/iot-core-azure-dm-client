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

using Newtonsoft.Json.Linq;

namespace Microsoft.Devices.Management.DMDataContract
{
    public class CertificatesDataContract
    {
        public static readonly string NotFound = "<not found>";
        public static readonly string SectionName = "certificates";

        public static readonly string JsonRootCATrustedCertificates_Root = "rootCATrustedCertificates_Root";
        public static readonly string JsonRootCATrustedCertificates_CA = "rootCATrustedCertificates_CA";
        public static readonly string JsonRootCATrustedCertificates_TrustedPublisher = "rootCATrustedCertificates_TrustedPublisher";
        public static readonly string JsonRootCATrustedCertificates_TrustedPeople = "rootCATrustedCertificates_TrustedPeople";

        public static readonly string JsonCertificateStore_CA_System = "certificateStore_CA_System";
        public static readonly string JsonCertificateStore_Root_System = "certificateStore_Root_System";
        public static readonly string JsonCertificateStore_My_User = "certificateStore_My_User";
        public static readonly string JsonCertificateStore_My_System = "certificateStore_My_System";

        public class ReportedProperties
        {
            public string rootCATrustedCertificates_Root;
            public string rootCATrustedCertificates_CA;
            public string rootCATrustedCertificates_TrustedPublisher;
            public string rootCATrustedCertificates_TrustedPeople;

            public string certificateStore_CA_System;
            public string certificateStore_Root_System;
            public string certificateStore_My_User;
            public string certificateStore_My_System;

            public void LoadFrom(JObject json)
            {
                rootCATrustedCertificates_Root = Utils.GetString(json, JsonRootCATrustedCertificates_Root, NotFound);
                rootCATrustedCertificates_CA = Utils.GetString(json, JsonRootCATrustedCertificates_CA, NotFound);
                rootCATrustedCertificates_TrustedPublisher = Utils.GetString(json, JsonRootCATrustedCertificates_TrustedPublisher, NotFound);
                rootCATrustedCertificates_TrustedPeople = Utils.GetString(json, JsonRootCATrustedCertificates_TrustedPeople, NotFound);

                certificateStore_CA_System = Utils.GetString(json, JsonCertificateStore_CA_System, NotFound);
                certificateStore_Root_System = Utils.GetString(json, JsonCertificateStore_Root_System, NotFound);
                certificateStore_My_User = Utils.GetString(json, JsonCertificateStore_My_User, NotFound);
                certificateStore_My_System = Utils.GetString(json, JsonCertificateStore_My_System, NotFound);
            }
        }
    }
}