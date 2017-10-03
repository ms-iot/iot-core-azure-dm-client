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
    public class WindowsTelemetryDataContract
    {
        public const string NotFound = "<not found>";
        public const string SectionName = "windowsTelemetry";

        public const string JsonLevel = "level";
        public const string JsonSecurity = "security";
        public const string JsonBasic = "basic";
        public const string JsonEnhanced = "enhanced";
        public const string JsonFull = "full";

        public class DesiredProperties
        {
            public string level;

            public string ToJsonString()
            {
                StringBuilder sb = new StringBuilder();
                sb.Append("\"" + SectionName + "\" : {\n");
                sb.Append("\"" + JsonLevel + "\" : \"" + level + "\"\n");
                sb.Append("}");
                return sb.ToString();
            }

            public static DesiredProperties FromJsonObject(JObject telemetryObject)
            {
                DesiredProperties desiredProperties = new DesiredProperties();
                desiredProperties.level = Utils.GetString(telemetryObject, JsonLevel, NotFound);
                return desiredProperties;
            }
        }

        public class ReportedProperties
        {
            public string level;

            public void LoadFrom(JObject json)
            {
                level = Utils.GetString(json, JsonLevel, NotFound);
            }
        }
    }
}