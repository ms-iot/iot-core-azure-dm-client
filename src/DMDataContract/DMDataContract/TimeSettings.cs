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
    public class TimeSettingsDataContract
    {
        public const string NotFound = "<not found>";
        public const string SectionName = "timeInfo";

        public const string JsonTrue = "true";
        public const string JsonFalse = "false";

        public const string JsonLocalTime = "localTime";
        public const string JsonNtpServer = "ntpServer";
        public const string JsonTimeZoneBias = "timeZoneBias";
        public const string JsonTimeZoneStandardBias = "timeZoneStandardBias";
        public const string JsonTimeZoneStandardDate = "timeZoneStandardDate";
        public const string JsonTimeZoneStandardName = "timeZoneStandardName";
        public const string JsonTimeZoneStandardDayOfWeek = "timeZoneStandardDayOfWeek";
        public const string JsonTimeZoneDaylightBias = "timeZoneDaylightBias";
        public const string JsonTimeZoneDaylightDate = "timeZoneDaylightDate";
        public const string JsonTimeZoneDaylightName = "timeZoneDaylightName";
        public const string JSonTimeZoneDaylightDayOfWeek = "timeZoneDaylightDayOfWeek";
        public const string JSonTimeZoneKeyName = "timeZoneKeyName";
        public const string JSonDynamicDaylightTimeDisabled = "dynamicDaylightTimeDisabled";

        public class DesiredProperties
        {
            public string ntpServer;
            public int timeZoneBias;
            public int timeZoneStandardBias;
            public string timeZoneStandardDate;
            public string timeZoneStandardName;
            public int timeZoneStandardDayOfWeek;
            public int timeZoneDaylightBias;
            public string timeZoneDaylightDate;
            public string timeZoneDaylightName;
            public int timeZoneDaylightDayOfWeek;
            public string timeZoneKeyName;
            public bool dynamicDaylightTimeDisabled;

            public static DesiredProperties FromJsonObject(JObject timeServiceObject)
            {
                DesiredProperties properties = new DesiredProperties();
                properties.ntpServer = Utils.GetString(timeServiceObject, JsonNtpServer, NotFound);
                properties.timeZoneBias = Utils.GetInt(timeServiceObject, JsonTimeZoneBias, 0);
                properties.timeZoneStandardBias = Utils.GetInt(timeServiceObject, JsonTimeZoneStandardBias, 0);
                properties.timeZoneStandardDate = Utils.GetDateTimeAsString(timeServiceObject, JsonTimeZoneStandardDate, NotFound);
                properties.timeZoneStandardName = Utils.GetString(timeServiceObject, JsonTimeZoneStandardName, NotFound);
                properties.timeZoneStandardDayOfWeek = Utils.GetInt(timeServiceObject, JsonTimeZoneStandardDayOfWeek, 0);
                properties.timeZoneDaylightBias = Utils.GetInt(timeServiceObject, JsonTimeZoneDaylightBias, 0);
                properties.timeZoneDaylightDate = Utils.GetDateTimeAsString(timeServiceObject, JsonTimeZoneDaylightDate, NotFound);
                properties.timeZoneDaylightName = Utils.GetString(timeServiceObject, JsonTimeZoneDaylightName, NotFound);
                properties.timeZoneDaylightDayOfWeek = Utils.GetInt(timeServiceObject, JSonTimeZoneDaylightDayOfWeek, 0);
                properties.timeZoneKeyName = Utils.GetString(timeServiceObject, JSonTimeZoneKeyName, NotFound);
                properties.dynamicDaylightTimeDisabled = Utils.GetBool(timeServiceObject, JSonDynamicDaylightTimeDisabled, false);
                return properties;
            }

            public string ToJsonString()
            {
                StringBuilder sb = new StringBuilder();
                sb.Append("\"" + SectionName + "\" : {\n");
                sb.Append("\"" + JsonNtpServer + "\" : \"" + ntpServer + "\",\n");
                sb.Append("\"" + JsonTimeZoneBias + "\" : " + timeZoneBias + ",\n");
                sb.Append("\"" + JsonTimeZoneStandardBias + "\" : " + timeZoneStandardBias + ",\n");
                sb.Append("\"" + JsonTimeZoneStandardDate + "\" : \"" + timeZoneStandardDate + "\",\n");
                sb.Append("\"" + JsonTimeZoneStandardName + "\" : \"" + timeZoneStandardName + "\",\n");
                sb.Append("\"" + JsonTimeZoneStandardDayOfWeek + "\" : " + timeZoneStandardDayOfWeek + ",\n");
                sb.Append("\"" + JsonTimeZoneDaylightBias + "\" : " + timeZoneDaylightBias + ",\n");
                sb.Append("\"" + JsonTimeZoneDaylightDate + "\" : \"" + timeZoneDaylightDate + "\",\n");
                sb.Append("\"" + JsonTimeZoneDaylightName + "\" : \"" + timeZoneDaylightName + "\",\n");
                sb.Append("\"" + JSonTimeZoneDaylightDayOfWeek + "\" : " + timeZoneDaylightDayOfWeek + ",\n");
                sb.Append("\"" + JSonTimeZoneKeyName + "\" : \"" + timeZoneKeyName + "\",\n");
                sb.Append("\"" + JSonDynamicDaylightTimeDisabled + "\" : " + (dynamicDaylightTimeDisabled ? JsonTrue : JsonFalse) + "\n");
                sb.Append("}");

                System.Diagnostics.Debug.WriteLine(sb.ToString());

                return sb.ToString();
            }
        }

        public class ReportedProperties
        {
            public string localTime;
            public string ntpServer;
            public bool dynamicDaylightTimeDisabled;
            public string timeZoneKeyName;
            public int timeZoneBias;
            public int timeZoneStandardBias;
            public string timeZoneStandardDate;
            public string timeZoneStandardName;
            public int timeZoneStandardDayOfWeek;
            public int timeZoneDaylightBias;
            public string timeZoneDaylightDate;
            public string timeZoneDaylightName;
            public int timeZoneDaylightDayOfWeek;

            public static ReportedProperties FromJsonObject(JObject timeServiceObject)
            {
                ReportedProperties properties = new ReportedProperties();

                properties.localTime = Utils.GetString(timeServiceObject, JsonLocalTime, NotFound);
                properties.ntpServer = Utils.GetString(timeServiceObject, JsonNtpServer, NotFound);

                properties.dynamicDaylightTimeDisabled = Utils.GetBool(timeServiceObject, JSonDynamicDaylightTimeDisabled, false);

                properties.timeZoneKeyName = Utils.GetString(timeServiceObject, JSonTimeZoneKeyName, NotFound);

                properties.timeZoneBias = Utils.GetInt(timeServiceObject, JsonTimeZoneBias, 0);
                properties.timeZoneStandardBias = Utils.GetInt(timeServiceObject, JsonTimeZoneStandardBias, 0);
                properties.timeZoneStandardDate = Utils.GetDateTimeAsString(timeServiceObject, JsonTimeZoneStandardDate, NotFound);
                properties.timeZoneStandardName = Utils.GetString(timeServiceObject, JsonTimeZoneStandardName, NotFound);
                properties.timeZoneStandardDayOfWeek = Utils.GetInt(timeServiceObject, JsonTimeZoneStandardDayOfWeek, 0);
                properties.timeZoneDaylightBias = Utils.GetInt(timeServiceObject, JsonTimeZoneDaylightBias, 0);
                properties.timeZoneDaylightDate = Utils.GetDateTimeAsString(timeServiceObject, JsonTimeZoneDaylightDate, NotFound);
                properties.timeZoneDaylightName = Utils.GetString(timeServiceObject, JsonTimeZoneDaylightName, NotFound);
                properties.timeZoneDaylightDayOfWeek = Utils.GetInt(timeServiceObject, JSonTimeZoneDaylightDayOfWeek, 0);
                return properties;
            }

            public JObject ToJsonObject()
            {
                JObject jObject = new JObject();
                jObject.Add(JsonLocalTime, new JValue(localTime));
                jObject.Add(JsonNtpServer, new JValue(ntpServer));

                jObject.Add(JSonDynamicDaylightTimeDisabled, new JValue(dynamicDaylightTimeDisabled));

                jObject.Add(JSonTimeZoneKeyName, new JValue(timeZoneKeyName));

                jObject.Add(JsonTimeZoneBias, new JValue(timeZoneBias));
                jObject.Add(JsonTimeZoneStandardBias, new JValue(timeZoneStandardBias));
                jObject.Add(JsonTimeZoneStandardDate, new JValue(timeZoneStandardDate));
                jObject.Add(JsonTimeZoneStandardName, new JValue(timeZoneStandardName));
                jObject.Add(JsonTimeZoneStandardDayOfWeek, new JValue(timeZoneStandardDayOfWeek));
                jObject.Add(JsonTimeZoneDaylightBias, new JValue(timeZoneDaylightBias));
                jObject.Add(JsonTimeZoneDaylightDate, new JValue(timeZoneDaylightDate));
                jObject.Add(JsonTimeZoneDaylightName, new JValue(timeZoneDaylightName));
                jObject.Add(JSonTimeZoneDaylightDayOfWeek, new JValue(timeZoneDaylightDayOfWeek));
                return jObject;
            }
        }
    }
}