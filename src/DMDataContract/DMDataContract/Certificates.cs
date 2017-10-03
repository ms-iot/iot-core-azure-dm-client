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
using System;
using System.Collections.Generic;
using System.Text;

namespace Microsoft.Devices.Management.DMDataContract
{
    public class CertificatesDataContract
    {
        public const string NotFound = "<not found>";
        public const string SectionName = "certificates";

        public const string JsonFileName = "fileName";
        public const string JsonState = "state";

        public const string JsonStateUnknown = "unknown";
        public const string JsonStateInstalled = "installed";
        public const string JsonStateUninstalled = "uninstalled";

        public const char JsonHashSeparator = '/';

        public const string JsonRootCATrustedCertificates_Root = "rootCATrustedCertificates_Root";
        public const string JsonRootCATrustedCertificates_CA = "rootCATrustedCertificates_CA";
        public const string JsonRootCATrustedCertificates_TrustedPublisher = "rootCATrustedCertificates_TrustedPublisher";
        public const string JsonRootCATrustedCertificates_TrustedPeople = "rootCATrustedCertificates_TrustedPeople";

        public const string JsonCertificateStore_CA_System = "certificateStore_CA_System";
        public const string JsonCertificateStore_Root_System = "certificateStore_Root_System";
        public const string JsonCertificateStore_My_User = "certificateStore_My_User";
        public const string JsonCertificateStore_My_System = "certificateStore_My_System";

        public class CertificateInfo
        {
            public string Hash { get; set; }
            public string StorageFileName { get; set; }
            public string State { get; set; }

            public void ToJson(StringBuilder sb)
            {
                sb.Append("\n        \"" + Hash + "\" : {");
                sb.Append("\n            \"" + JsonFileName + "\": \"" + StorageFileName + "\",");
                sb.Append("\n            \"" + JsonState + "\": \"" + State + "\"");
                sb.Append("\n        }");
            }

            public static CertificateInfo FromJson(JProperty property)
            {
                CertificateInfo info = new CertificateInfo();
                info.Hash = property.Name;

                if (!(property.Value is JObject))
                {
                    throw new Exception("Certificate hash values are expected to be objects.");
                }

                JObject objectValue = (JObject)property.Value;
                info.StorageFileName = Utils.GetString(objectValue, JsonFileName, NotFound);
                info.State = Utils.GetString(objectValue, JsonState, NotFound);
                return info;
            }
        }

        public class DesiredProperties
        {
            public List<CertificateInfo> rootCATrustedCertificates_Root;
            public List<CertificateInfo> rootCATrustedCertificates_CA;
            public List<CertificateInfo> rootCATrustedCertificates_TrustedPublisher;
            public List<CertificateInfo> rootCATrustedCertificates_TrustedPeople;

            public List<CertificateInfo> certificateStore_CA_System;
            public List<CertificateInfo> certificateStore_Root_System;
            public List<CertificateInfo> certificateStore_My_User;
            public List<CertificateInfo> certificateStore_My_System;

            public DesiredProperties()
            {
                rootCATrustedCertificates_Root = new List<CertificateInfo>();
                rootCATrustedCertificates_CA = new List<CertificateInfo>();
                rootCATrustedCertificates_TrustedPublisher = new List<CertificateInfo>();
                rootCATrustedCertificates_TrustedPeople = new List<CertificateInfo>();

                certificateStore_CA_System = new List<CertificateInfo>();
                certificateStore_Root_System = new List<CertificateInfo>();
                certificateStore_My_User = new List<CertificateInfo>();
                certificateStore_My_System = new List<CertificateInfo>();
            }

            private void ToJson(StringBuilder sb, string nodeName, List<CertificateInfo> state)
            {
                sb.Append("\n    \"" + nodeName + "\" : {");
                bool first = true;
                foreach (CertificateInfo certificateInfo in state)
                {
                    if (!first)
                    {
                        sb.Append(",");
                    }
                    certificateInfo.ToJson(sb);
                    first = false;
                }
                sb.Append("\n    }");
            }

            public string ToJsonString()
            {
                StringBuilder sb = new StringBuilder();
                sb.Append("\"" + SectionName + "\" : {\n");
                ToJson(sb, JsonRootCATrustedCertificates_Root, rootCATrustedCertificates_Root);
                sb.Append(",");
                ToJson(sb, JsonRootCATrustedCertificates_CA, rootCATrustedCertificates_CA);
                sb.Append(",");
                ToJson(sb, JsonRootCATrustedCertificates_TrustedPublisher, rootCATrustedCertificates_TrustedPublisher);
                sb.Append(",");
                ToJson(sb, JsonRootCATrustedCertificates_TrustedPeople, rootCATrustedCertificates_TrustedPeople);
                sb.Append(",");
                ToJson(sb, JsonCertificateStore_CA_System, certificateStore_CA_System);
                sb.Append(",");
                ToJson(sb, JsonCertificateStore_Root_System, certificateStore_Root_System);
                sb.Append(",");
                ToJson(sb, JsonCertificateStore_My_User, certificateStore_My_User);
                sb.Append(",");
                ToJson(sb, JsonCertificateStore_My_System, certificateStore_My_System);
                sb.Append("}");
                return sb.ToString();
            }

            private static List<CertificateInfo> FromJsonObject(JObject root, string storeName)
            {
                JToken storeNode = root[storeName];

                if (!(storeNode is JObject))
                {
                    throw new Exception("Unexpected type for certificate store node!");
                }
                
                List<CertificateInfo> certificateList = new List<CertificateInfo>();

                foreach (JProperty property in storeNode.Children())
                {
                    CertificateInfo info = CertificateInfo.FromJson(property);
                    certificateList.Add(info);
                }
                return certificateList;
            }

            public static DesiredProperties FromJsonObject(JObject root)
            {
                DesiredProperties desiredProperties = new DesiredProperties();

                desiredProperties.rootCATrustedCertificates_Root = FromJsonObject(root, JsonRootCATrustedCertificates_Root);
                desiredProperties.rootCATrustedCertificates_CA = FromJsonObject(root, JsonRootCATrustedCertificates_CA);
                desiredProperties.rootCATrustedCertificates_TrustedPublisher = FromJsonObject(root, JsonRootCATrustedCertificates_TrustedPublisher);
                desiredProperties.rootCATrustedCertificates_TrustedPeople = FromJsonObject(root, JsonRootCATrustedCertificates_TrustedPeople);

                desiredProperties.certificateStore_CA_System = FromJsonObject(root, JsonCertificateStore_CA_System);
                desiredProperties.certificateStore_Root_System = FromJsonObject(root, JsonCertificateStore_Root_System);
                desiredProperties.certificateStore_My_User = FromJsonObject(root, JsonCertificateStore_My_User);
                desiredProperties.certificateStore_My_System = FromJsonObject(root, JsonCertificateStore_My_System);

                return desiredProperties;
            }
        }

        public class ReportedProperties
        {
            public List<string> rootCATrustedCertificates_Root;
            public List<string> rootCATrustedCertificates_CA;
            public List<string> rootCATrustedCertificates_TrustedPublisher;
            public List<string> rootCATrustedCertificates_TrustedPeople;

            public List<string> certificateStore_CA_System;
            public List<string> certificateStore_Root_System;
            public List<string> certificateStore_My_User;
            public List<string> certificateStore_My_System;

            public void Set(string storeName, string stringValue)
            {
                List<string> list;
                if (String.IsNullOrEmpty(stringValue))
                {
                    list = new List<string>();
                }
                else
                {
                    list = new List<string>(stringValue.Split(JsonHashSeparator));
                }

                switch (storeName)
                {
                    case JsonRootCATrustedCertificates_Root:
                        rootCATrustedCertificates_Root = list;
                        break;
                    case JsonRootCATrustedCertificates_CA:
                        rootCATrustedCertificates_CA = list;
                        break;
                    case JsonRootCATrustedCertificates_TrustedPublisher:
                        rootCATrustedCertificates_TrustedPublisher = list;
                        break;
                    case JsonRootCATrustedCertificates_TrustedPeople:
                        rootCATrustedCertificates_TrustedPeople = list;
                        break;
                    case JsonCertificateStore_CA_System:
                        certificateStore_CA_System = list;
                        break;
                    case JsonCertificateStore_Root_System:
                        certificateStore_Root_System = list;
                        break;
                    case JsonCertificateStore_My_User:
                        certificateStore_My_User = list;
                        break;
                    case JsonCertificateStore_My_System:
                        certificateStore_My_System = list;
                        break;
                }
            }

            private JObject ToJsonObject(IEnumerable<string> hashes)
            {
                JObject jObject = new JObject();

                foreach (string hash in hashes)
                {
                    jObject.Add(hash, new JValue(""));
                }

                return jObject;
            }

            public JObject ToJsonObject()
            {
                JObject jObject = new JObject();

                jObject.Add(JsonRootCATrustedCertificates_Root, ToJsonObject(rootCATrustedCertificates_Root));
                jObject.Add(JsonRootCATrustedCertificates_CA, ToJsonObject(rootCATrustedCertificates_CA));
                jObject.Add(JsonRootCATrustedCertificates_TrustedPublisher, ToJsonObject(rootCATrustedCertificates_TrustedPublisher));
                jObject.Add(JsonRootCATrustedCertificates_TrustedPeople, ToJsonObject(rootCATrustedCertificates_TrustedPeople));

                jObject.Add(JsonCertificateStore_CA_System, ToJsonObject(certificateStore_CA_System));
                jObject.Add(JsonCertificateStore_Root_System, ToJsonObject(certificateStore_Root_System));
                jObject.Add(JsonCertificateStore_My_User, ToJsonObject(certificateStore_My_User));
                jObject.Add(JsonCertificateStore_My_System, ToJsonObject(certificateStore_My_System));
                
                return jObject;
            }

            private static List<string> FromJsonObject(JObject root, string storeName)
            {
                JToken storeNode = root[storeName];

                if (!(storeNode is JObject))
                {
                    throw new Exception("Unexpected type for certificate store node!");
                }

                List<string> hashList = new List<string>();

                foreach (JProperty property in storeNode.Children())
                {
                    hashList.Add(property.Name);
                }
                return hashList;
            }

            public static ReportedProperties FromJsonObject(JObject root)
            {
                ReportedProperties reportedProperties = new ReportedProperties();

                reportedProperties.rootCATrustedCertificates_Root = FromJsonObject(root, JsonRootCATrustedCertificates_Root);
                reportedProperties.rootCATrustedCertificates_CA = FromJsonObject(root, JsonRootCATrustedCertificates_CA);
                reportedProperties.rootCATrustedCertificates_TrustedPublisher = FromJsonObject(root, JsonRootCATrustedCertificates_TrustedPublisher);
                reportedProperties.rootCATrustedCertificates_TrustedPeople = FromJsonObject(root, JsonRootCATrustedCertificates_TrustedPeople);

                reportedProperties.certificateStore_CA_System = FromJsonObject(root, JsonCertificateStore_CA_System);
                reportedProperties.certificateStore_Root_System = FromJsonObject(root, JsonCertificateStore_Root_System);
                reportedProperties.certificateStore_My_User = FromJsonObject(root, JsonCertificateStore_My_User);
                reportedProperties.certificateStore_My_System = FromJsonObject(root, JsonCertificateStore_My_System);

                return reportedProperties;
            }
        }
    }
}