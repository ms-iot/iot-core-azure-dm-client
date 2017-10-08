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
using System.Text;

namespace Microsoft.Devices.Management.DMDataContract
{
    public class RebootInfoDataContract
    {
        public static readonly string NotFound = "<not found>";
        public static readonly string SectionName = "rebootInfo";

        public static readonly string JsonSingleRebootTime = "singleRebootTime";
        public static readonly string JsonDailyRebootTime = "dailyRebootTime";
        public static readonly string JsonLastBootTime = "lastBootTime";

        public class DesiredProperties
        {
            public string singleRebootTime;
            public string dailyRebootTime;

            public static DesiredProperties FromJsonObject(JObject root)
            {
                DesiredProperties desiredProperties = new DesiredProperties();
                desiredProperties.singleRebootTime = Utils.GetDateTimeAsString(root, JsonSingleRebootTime, "");
                desiredProperties.dailyRebootTime = Utils.GetDateTimeAsString(root, JsonDailyRebootTime, "");
                return desiredProperties;
            }

            public string ToJsonString()
            {
                StringBuilder sb = new StringBuilder();
                sb.Append("\"" + SectionName + "\" : {\n");
                sb.Append("\"" + JsonSingleRebootTime + "\" : \"" + singleRebootTime + "\",\n");
                sb.Append("\"" + JsonDailyRebootTime + "\" : \"" + dailyRebootTime + "\"\n");
                sb.Append("}");
                return sb.ToString();
            }
        }

        public class ReportedProperties
        {
            public string lastBootTime;
            public string singleRebootTime;
            public string dailyRebootTime;

            public static ReportedProperties FromJsonObject(JObject root)
            {
                ReportedProperties reportedProperties = new ReportedProperties();
                reportedProperties.singleRebootTime = Utils.GetDateTimeAsString(root, JsonSingleRebootTime, NotFound);
                reportedProperties.dailyRebootTime = Utils.GetDateTimeAsString(root, JsonDailyRebootTime, NotFound);
                reportedProperties.lastBootTime = Utils.GetDateTimeAsString(root, JsonLastBootTime, NotFound);
                return reportedProperties;
            }

            public JObject ToJsonObject()
            {
                return JObject.FromObject(this);
            }
        }
    }
}