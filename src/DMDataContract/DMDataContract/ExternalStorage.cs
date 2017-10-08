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
    public class ExternalStorageDataContract
    {
        public const string NotFound = "<not found>";
        public const string SectionName = "externalStorage";

        public const string JsonConnectionString = "connectionString";
        public const string JsonNotSet = "<not set>";

        public class DesiredProperties
        {
            public string connectionString;

            public static DesiredProperties FromJsonObject(JObject externalStorage)
            {
                DesiredProperties desiredProperties = new DesiredProperties();
                desiredProperties.connectionString = Utils.GetString(externalStorage, JsonConnectionString, "");
                return desiredProperties;
            }

            public string ToJsonString()
            {
                StringBuilder sb = new StringBuilder();
                sb.Append("\"" + SectionName + "\" : {\n");
                sb.Append("\"" + JsonConnectionString + "\" : \"" + connectionString + "\"\n");
                sb.Append("}");
                return sb.ToString();
            }
        }

        public class ReportedProperties
        {
            public string connectionString;

            public static ReportedProperties FromJsonObject(JObject json)
            {
                ReportedProperties reportedProperties = new ReportedProperties();
                reportedProperties.connectionString = Utils.GetString(json, JsonConnectionString, NotFound);
                return reportedProperties;
            }

            public JObject ToJsonObject()
            {
                JObject jObject = new JObject();
                jObject.Add(JsonConnectionString, new JValue(connectionString));
                return jObject;
            }
        }
    }
}