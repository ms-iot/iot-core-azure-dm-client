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
using System.Text;

namespace Microsoft.Devices.Management.DMDataContract
{
    public class WindowsUpdatesDataContract
    {
        public const string NotFound = "<not found>";
        public const string Unexpected = "<unexpected>";
        public const string SectionName = "windowsUpdates";

        public const string JsonApproved = "approved";
        public const string JsonInstalled = "installed";
        public const string JsonFailed = "failed";
        public const string JsonInstallable = "installable";
        public const string JsonPendingReboot = "pendingReboot";
        public const string JsonLastScanTime = "lastScanTime";
        public const string JsonDeferUpgrade = "deferUpgrade";

        public class DesiredProperties
        {
            public string approved;

            public string ToJsonString()
            {
                StringBuilder sb = new StringBuilder();
                sb.Append("\"" + SectionName + "\" : {\n");
                sb.Append("    \"" + JsonApproved + "\": \"" + approved + "\"");
                sb.Append("}");
                return sb.ToString();
            }

            public static DesiredProperties FromJsonObject(JObject root)
            {
                DesiredProperties desiredProperties = new DesiredProperties();
                desiredProperties.approved = Utils.GetString(root, JsonApproved, NotFound);
                return desiredProperties;
            }
        }

        public class ReportedProperties
        {
            public string approved;
            public string installed;
            public string failed;
            public string installable;
            public string pendingReboot;
            public string lastScanTime;
            public bool deferUpgrade;

            public JObject ToJsonObject()
            {
                JObject jObject = new JObject();

                jObject.Add(JsonApproved, new JValue(approved));
                jObject.Add(JsonInstalled, new JValue(installed));
                jObject.Add(JsonFailed, new JValue(failed));
                jObject.Add(JsonInstallable, new JValue(installable));
                jObject.Add(JsonPendingReboot, new JValue(pendingReboot));
                jObject.Add(JsonLastScanTime, new JValue(lastScanTime));
                jObject.Add(JsonDeferUpgrade, new JValue(deferUpgrade));

                return jObject;
            }

            public static ReportedProperties FromJsonObject(JObject root)
            {
                ReportedProperties reportedProperties = new ReportedProperties();

                reportedProperties.approved = Utils.GetString(root, JsonApproved, NotFound);
                reportedProperties.installed = Utils.GetString(root, JsonInstalled, NotFound);
                reportedProperties.failed = Utils.GetString(root, JsonFailed, NotFound);
                reportedProperties.installable = Utils.GetString(root, JsonInstallable, NotFound);
                reportedProperties.pendingReboot = Utils.GetString(root, JsonPendingReboot, NotFound);
                reportedProperties.lastScanTime = Utils.GetString(root, JsonLastScanTime, NotFound);
                reportedProperties.deferUpgrade = Utils.GetBool(root, JsonDeferUpgrade, false);

                return reportedProperties;
            }

        }
    }
}
