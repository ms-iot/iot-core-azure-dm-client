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
        public static readonly string NotFound = "<not found>";
        public static readonly string SectionName = "windowsTelemetry";

        public static readonly string JsonLevel = "level";
        public static readonly string JsonSecurity = "security";
        public static readonly string JsonBasic = "basic";
        public static readonly string JsonEnhanced = "enhanced";
        public static readonly string JsonFull = "full";

        public class DesiredProperties
        {
            public string level;

            public void LoadFrom(JObject telemetryObject)
            {
                level = Utils.GetString(telemetryObject, JsonLevel, "");
            }

            public string ToJson()
            {
                StringBuilder sb = new StringBuilder();
                sb.Append("\"" + SectionName + "\" : {\n");
                sb.Append("\"" + JsonLevel + "\" : \"" + level + "\"\n");
                sb.Append("}");
                return sb.ToString();
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