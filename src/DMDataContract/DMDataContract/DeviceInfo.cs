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
    public class DeviceInfoDataContract
    {
        public static readonly string NotFound = "<not found>";
        public static readonly string SectionName = "deviceInfo";

        public static readonly string JsonBatteryRuntime = "batteryRuntime";
        public static readonly string JsonBatteryRemaining= "batteryRemaining";
        public static readonly string JsonBatteryStatus= "batteryStatus";
        public static readonly string JsonOsEdition= "osEdition";
        public static readonly string JsonSecureBootState= "secureBootState";

        public static readonly string JsonTotalMemory = "totalMemory";
        public static readonly string JsonTotalStorage= "totalStorage";
        public static readonly string JsonName= "name";
        public static readonly string JsonProcessorArchitecture = "processorArchitecture";
        public static readonly string JsonCommercializationOperator = "commercializationOperator";

        public static readonly string JsonDisplayResolution= "displayResolution";
        public static readonly string JsonRadioSwVer = "radioSwVer";
        public static readonly string JsonProcessorType= "processorType";
        public static readonly string JsonPlatform= "platform";
        public static readonly string JsonOsVer = "osVer";

        public static readonly string JsonFwVer = "fwVer";
        public static readonly string JsonHwVer = "hwVer";
        public static readonly string JsonOEM = "oem";
        public static readonly string JsonType = "type";
        public static readonly string JsonLang = "lang";

        public static readonly string JsonDmVer = "dmVer";
        public static readonly string JsonModel = "Model";
        public static readonly string JsonManufacturer = "manufacturer";
        public static readonly string JsonId = "id";

        public class DesiredProperties
        {
            public string ToJsonString()
            {
                StringBuilder sb = new StringBuilder();
                sb.Append("\"" + SectionName + "\" : {\n");
                sb.Append("}");
                return sb.ToString();
            }
        }

        public class ReportedProperties
        {
            public string batteryRuntime;
            public string batteryRemaining;
            public string batteryStatus;
            public string osEdition;
            public string secureBootState;
            public string totalMemory;
            public string totalStorage;
            public string name;
            public string processorArchitecture;
            public string commercializationOperator;
            public string displayResolution;
            public string radioSwVer;
            public string processorType;
            public string platform;
            public string osVer;
            public string fwVer;
            public string hwVer;
            public string oem;
            public string type;
            public string lang;
            public string dmVer;
            public string model;
            public string manufacturer;
            public string id;

            public static ReportedProperties FromJsonObject(JObject json)
            {
                ReportedProperties reportedProperties = new ReportedProperties();

                reportedProperties.batteryRuntime = Utils.GetString(json, JsonBatteryRuntime, NotFound);
                reportedProperties.batteryRemaining = Utils.GetString(json, JsonBatteryRemaining, NotFound);
                reportedProperties.batteryStatus = Utils.GetString(json, JsonBatteryStatus, NotFound);
                reportedProperties.osEdition = Utils.GetString(json, JsonOsEdition, NotFound);
                reportedProperties.secureBootState = Utils.GetString(json, JsonSecureBootState, NotFound);

                reportedProperties.totalMemory = Utils.GetString(json, JsonTotalMemory, NotFound);
                reportedProperties.totalStorage = Utils.GetString(json, JsonTotalStorage, NotFound);
                reportedProperties.name = Utils.GetString(json, JsonName, NotFound);
                reportedProperties.processorArchitecture = Utils.GetString(json, JsonProcessorArchitecture, NotFound);
                reportedProperties.commercializationOperator = Utils.GetString(json, JsonCommercializationOperator, NotFound);

                reportedProperties.displayResolution = Utils.GetString(json, JsonDisplayResolution, NotFound);
                reportedProperties.radioSwVer = Utils.GetString(json, JsonRadioSwVer, NotFound);
                reportedProperties.processorType = Utils.GetString(json, JsonProcessorType, NotFound);
                reportedProperties.platform = Utils.GetString(json, JsonPlatform, NotFound);
                reportedProperties.osVer = Utils.GetString(json, JsonOsVer, NotFound);

                reportedProperties.fwVer = Utils.GetString(json, JsonFwVer, NotFound);
                reportedProperties.hwVer = Utils.GetString(json, JsonHwVer, NotFound);
                reportedProperties.oem = Utils.GetString(json, JsonOEM, NotFound);
                reportedProperties.type = Utils.GetString(json, JsonType, NotFound);
                reportedProperties.lang = Utils.GetString(json, JsonLang, NotFound);

                reportedProperties.dmVer = Utils.GetString(json, JsonDmVer, NotFound);
                reportedProperties.model = Utils.GetString(json, JsonModel, NotFound);
                reportedProperties.manufacturer = Utils.GetString(json, JsonManufacturer, NotFound);
                reportedProperties.id = Utils.GetString(json, JsonId, NotFound);

                return reportedProperties;
            }

            public JObject ToJsonObject()
            {
                return JObject.FromObject(this);
            }
        }
    }
}