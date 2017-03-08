using System;
using Newtonsoft.Json.Linq;
using System.Diagnostics;
using Microsoft.Devices.Management;
using Microsoft.Devices.Management.Message;
using System.Collections.Generic;
using System.Threading.Tasks;

namespace IoTDMClient
{
    class WindowsUpdatesManagement
    {
        public static SetWindowsUpdatePolicyRequest GetDesiredWindowsUpdatePolicyRequest(JProperty configurationJson)
        {
            if (configurationJson.Value.Type != JTokenType.Object)
            {
                throw new Exception("Invalid json object for configuration.");
            }

            Debug.WriteLine("windows update policy = " + configurationJson.Value.ToString());

            JObject subProperties = (JObject)configurationJson.Value;

            var configuration = new WindowsUpdatePolicyConfiguration();

            configuration.activeHoursStart = (uint)subProperties.Property("activeHoursStart").Value;
            configuration.activeHoursEnd = (uint)subProperties.Property("activeHoursEnd").Value;
            configuration.allowAutoUpdate = (uint)subProperties.Property("allowAutoUpdate").Value;
            configuration.allowMUUpdateService = (uint)subProperties.Property("allowMUUpdateService").Value;
            configuration.allowNonMicrosoftSignedUpdate = (uint)subProperties.Property("allowNonMicrosoftSignedUpdate").Value;

            configuration.allowUpdateService = (uint)subProperties.Property("allowUpdateService").Value;
            configuration.branchReadinessLevel = (uint)subProperties.Property("branchReadinessLevel").Value;
            configuration.deferFeatureUpdatesPeriod = (uint)subProperties.Property("deferFeatureUpdatesPeriod").Value;
            configuration.deferQualityUpdatesPeriod = (uint)subProperties.Property("deferQualityUpdatesPeriod").Value;
            configuration.excludeWUDrivers = (uint)subProperties.Property("excludeWUDrivers").Value;

            configuration.pauseFeatureUpdates = (uint)subProperties.Property("pauseFeatureUpdates").Value;
            configuration.pauseQualityUpdates = (uint)subProperties.Property("pauseQualityUpdates").Value;
            configuration.requireUpdateApproval = (uint)subProperties.Property("requireUpdateApproval").Value;
            configuration.scheduledInstallDay = (uint)subProperties.Property("scheduledInstallDay").Value;
            configuration.scheduledInstallTime = (uint)subProperties.Property("scheduledInstallTime").Value;

            configuration.updateServiceUrl = (string)subProperties.Property("updateServiceUrl").Value;

            return new SetWindowsUpdatePolicyRequest(configuration);
        }

        public static SetWindowsUpdateRebootPolicyRequest GetDesiredWindowsUpdateRebootPolicyRequest(JProperty configurationJson)
        {
            if (configurationJson.Value.Type != JTokenType.Object)
            {
                throw new Exception("Invalid json object for configuration.");
            }

            Debug.WriteLine("windows update reboot policy = " + configurationJson.Value.ToString());

            JObject subProperties = (JObject)configurationJson.Value;

            var configuration = new WindowsUpdateRebootPolicyConfiguration();

            configuration.allow = (bool)subProperties.Property("allow").Value;

            return new SetWindowsUpdateRebootPolicyRequest(configuration);
        }

        public static SetWindowsUpdatesRequest GetDesiredWindowsUpdatesRequest(JProperty configurationJson)
        {
            if (configurationJson.Value.Type != JTokenType.Object)
            {
                throw new Exception("Invalid json object for configuration.");
            }

            Debug.WriteLine("windows updates = " + configurationJson.Value.ToString());

            JObject subProperties = (JObject)configurationJson.Value;
            var configuration = new SetWindowsUpdatesConfiguration();
            configuration.approved = (string)subProperties.Property("approved").Value;
            return new SetWindowsUpdatesRequest(configuration);
        }

    }
}