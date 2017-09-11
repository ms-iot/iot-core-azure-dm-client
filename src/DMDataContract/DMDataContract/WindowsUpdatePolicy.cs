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
    public class WindowsUpdatePolicyDataContract
    {
        public const string NotFound = "<not found>";
        public const string Unexpected = "<unexpected>";
        public const string SectionName = "windowsUpdatePolicy";

        public const string JsonYes = "yes";
        public const string JsonNo = "no";

        public const string JsonActiveHoursStart = "activeHoursStart";
        public const string JsonActiveHoursEnd = "activeHoursEnd";
        public const string JsonAllowAutoUpdate = "allowAutoUpdate";
        public const string JsonAllowUpdateService = "allowUpdateService";
        public const string JsonBranchReadinessLevel = "branchReadinessLevel";

        public const string JsonDeferFeatureUpdatesPeriod = "deferFeatureUpdatesPeriod";
        public const string JsonDeferQualityUpdatesPeriod = "deferQualityUpdatesPeriod";
        public const string JsonPauseFeatureUpdates = "pauseFeatureUpdates";
        public const string JsonPauseQualityUpdates = "pauseQualityUpdates";
        public const string JsonScheduledInstallDay = "scheduledInstallDay";

        public const string JsonScheduledInstallTime = "scheduledInstallTime";

        public const string JsonRing = "ring";
        public const string JsonEarlyAdopter = "EarlyAdopter";
        public const string JsonPreview = "Preview";
        public const string JsonGeneralAvailability = "GeneralAvailability";

        public const string JsonApplyProperties = "applyProperties";
        public const string JsonReportProperties = "reportProperties";

        public const string JsonSourcePriority = "sourcePriority";

        public class WUProperties
        {
            public string activeHoursStart;
            public string activeHoursEnd;
            public string allowAutoUpdate;
            public string allowUpdateService;
            public string branchReadinessLevel;

            public string deferFeatureUpdatesPeriod;    // in days
            public string deferQualityUpdatesPeriod;    // in days
            public string pauseFeatureUpdates;
            public string pauseQualityUpdates;
            public string scheduledInstallDay;
            public string scheduledInstallTime;

            public string ring;

            public string sourcePriority;

            public void LoadFrom(JObject json)
            {
                activeHoursStart = Utils.GetString(json, JsonActiveHoursStart, NotFound);
                activeHoursEnd = Utils.GetString(json, JsonActiveHoursEnd, NotFound);
                allowAutoUpdate = Utils.GetString(json, JsonAllowAutoUpdate, NotFound);
                allowUpdateService = Utils.GetString(json, JsonAllowUpdateService, NotFound);
                branchReadinessLevel = Utils.GetString(json, JsonBranchReadinessLevel, NotFound);

                deferFeatureUpdatesPeriod = Utils.GetString(json, JsonDeferFeatureUpdatesPeriod, NotFound);
                deferQualityUpdatesPeriod = Utils.GetString(json, JsonDeferQualityUpdatesPeriod, NotFound);
                pauseFeatureUpdates = Utils.GetString(json, JsonPauseFeatureUpdates, NotFound);
                pauseQualityUpdates = Utils.GetString(json, JsonPauseQualityUpdates, NotFound);
                scheduledInstallDay = Utils.GetString(json, JsonScheduledInstallDay, NotFound);

                scheduledInstallTime = Utils.GetString(json, JsonScheduledInstallTime, NotFound);

                ring = Utils.GetString(json, JsonRing, NotFound);

                sourcePriority = Utils.GetString(json, JsonSourcePriority, NotFound);
            }

            public JObject ToJson()
            {
                return JObject.FromObject(this);
            }
        }

        public class DesiredProperties
        {
            public WUProperties applyProperties;
            public string reportProperties;

            public void LoadFrom(JObject jObj)
            {
                JToken jValue;
                if (jObj.TryGetValue(JsonApplyProperties, out jValue) && jValue is JObject)
                {
                    applyProperties = new WUProperties();
                    applyProperties.LoadFrom((JObject)jValue);
                }
                if (jObj.TryGetValue(JsonReportProperties, out jValue) && jValue is JValue && jValue.Type == JTokenType.String)
                {
                    reportProperties = (string)jValue;
                }
            }
        }
    }
}