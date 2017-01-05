using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading.Tasks;

namespace Microsoft.Devices.Management
{
    public class StartupAppInfo
    {
        public string AppId { get; set; }
        public bool IsBackgroundApplication { get; set; }
    }

    public class AppInfo
    {
        public string AppSource { get; set; }
        public string Architecture { get; set; }
        public string InstallDate { get; set; }
        public string InstallLocation { get; set; }
        public string IsBundle { get; set; }
        public string IsFramework { get; set; }
        public string IsProvisioned { get; set; }
        public string Name { get; set; }
        public string PackageFamilyName { get; set; }
        public string PackageStatus { get; set; }
        public string Publisher { get; set; }
        public string RequiresReinstall { get; set; }
        public string ResourceID { get; set; }
        public string Users { get; set; }
        public string Version { get; set; }

        public static Dictionary<string, AppInfo> SetOfAppsFromJson(string json)
        {
            return JsonConvert.DeserializeObject<Dictionary<string, AppInfo>>(json);
        }
    }

    public class AppxInstallInfo
    {
        public string PackageFamilyName { get; set; }
        public string AppxPath { get; set; }
        public List<string> Dependencies { get; set; }

        public AppxInstallInfo()
        {
            Dependencies = new List<string>();
        }
    }

    public class AppxUninstallInfo
    {
        public string PackageFamilyName { get; set; }
        public bool StoreApp { get; set; }
    }

    // TODO: this type will come from the Azure IoT SDK
    public enum DeviceTwinUpdateState
    {
        Complete,
        Partial,
    }

    // This is the main entry point into DM
    public class DeviceManagementClient
    {
        // Constants
        public const string RebootMethod = "Reboot";
        public const string FactoryResetMethod = "FactoryReset";

        // device twin property paths
        public const string DesiredRebootSingleProperty = "properties.desired.reboot.singleReboot";
        public const string DesiredRebootDailyProperty = "properties.desired.reboot.dailyReboot";

        public const string ReportedRebootSingleProperty = "properties.reported.reboot.singleReboot";
        public const string ReportedRebootDailyProperty = "properties.reported.reboot.dailyReboot";
        public const string ReportedLastRebootCmdProperty = "properties.reported.reboot.lastRebootCmd";
        public const string ReportedLastRebootProperty = "properties.reported.reboot.lastReboot";

        public const string ReportedTimeInfoProperty = "properties.reported.timeInfo";
        public const string DesiredTimeInfoProperty = "properties.desired.timeInfo";

        public const string ReportedDeviceStatusProperty = "properties.reported.deviceStatus";

        // Types
        public struct DMMethodResult
        {
            public uint returnCode;
            public string response;
        }

        public class TimeZoneInformation
        {
            public long bias;
            public string standardName;
            public DateTime standardDate;
            public long standardBias;
            public string daylightName;
            public DateTime daylightDate;
            public long daylightBias;
        }

        public class TimeInfo
        {
            public TimeInfo()
            {
                timeZoneInformation = new TimeZoneInformation();
            }

            public DateTime localTime;
            public string ntpServer;
            public TimeZoneInformation timeZoneInformation;
        }

        public struct DeviceStatus
        {
            public long secureBootState;
            public string macAddressIpV4;
            public string macAddressIpV6;
            public bool macAddressIsConnected;
            public long macAddressType;
            public string osType;
            public long batteryStatus;
            public long batteryRemaining;
            public long batteryRuntime;
        }

        // Ultimately, DeviceManagementClient will take an abstraction over DeviceClient to allow it to 
        // send reported properties. It will never receive using it
        private DeviceManagementClient(IDeviceTwin deviceTwin, IDeviceManagementRequestHandler requestHandler)
        {
            this.requestHandler = requestHandler;
            this.deviceTwin = deviceTwin;
            this.supportedMethods = new Dictionary<string, Func<string, Task<DMMethodResult>>>();
            this.supportedMethods.Add(RebootMethod, HandleRebootAsync);
            this.supportedMethods.Add(FactoryResetMethod, HandleFactoryResetAsync);

            this.supportedProperties = new Dictionary<string, DMCommand>();
            this.supportedProperties.Add(DesiredRebootSingleProperty, DMCommand.SetSingleRebootTime);
            this.supportedProperties.Add(ReportedRebootSingleProperty, DMCommand.GetSingleRebootTime);

            this.supportedProperties.Add(DesiredRebootDailyProperty, DMCommand.SetDailyRebootTime);
            this.supportedProperties.Add(ReportedRebootDailyProperty, DMCommand.GetDailyRebootTime);

            this.supportedProperties.Add(ReportedLastRebootCmdProperty, DMCommand.GetLastRebootCmdTime);
            this.supportedProperties.Add(ReportedLastRebootProperty, DMCommand.GetLastRebootTime);

            this.supportedProperties.Add(ReportedTimeInfoProperty, DMCommand.GetTimeInfo);
            this.supportedProperties.Add(DesiredTimeInfoProperty, DMCommand.SetTimeInfo);

            this.supportedProperties.Add(ReportedDeviceStatusProperty, DMCommand.GetDeviceStatus);
        }

        public static DeviceManagementClient Create(IDeviceTwin deviceTwin, IDeviceManagementRequestHandler requestHandler)
        {
            return new DeviceManagementClient(deviceTwin, requestHandler);
        }

        public bool IsDMMethod(string methodName)
        {
            return supportedMethods.ContainsKey(methodName);
        }

        public async Task<DMMethodResult> InvokeMethodAsync(string methodName, string payload)
        {
            if (!IsDMMethod(methodName))
            {
                throw new ArgumentException("Unknown method name: " + methodName);
            }

            return await supportedMethods[methodName](payload);
        }

        public bool IsDMProperty(string propertyName)
        {
            return supportedProperties.ContainsKey(propertyName);
        }

        public async Task SetPropertyAsync(string path, string valueString)
        {
            if (!IsDMProperty(path))
            {
                throw new ArgumentException("Unknown property name: " + path);
            }

            var request = new DMMessage(supportedProperties[path]);
            request.SetData(valueString);

            var result = await SystemConfiguratorProxy.SendCommandAsync(request);
            if (result.Context != 0)
            {
                throw new Exception();
            }
        }

        public async Task<string> GetPropertyAsync(string path)
        {
            var request = new DMMessage(supportedProperties[path]);
            var result = await SystemConfiguratorProxy.SendCommandAsync(request);
            if (result.Context != 0)
            {
                throw new Exception();
            }
            return result.GetDataString();
        }

        public async Task<TimeInfo> GetTimeInfoAsync()
        {
            /*
                {
                    "timeInfo":
                    {
                        "ntpServer": "pool.ntp.org",
                        "timeZone" :
                        {
                            "bias": 123,
                            "standardName": "(UTC-05:00) Eastern Time (US & Canada)",
                            "standardDate": "yyyy-mm-ddThh:mm:ss,day_of_week",
                            "standardBias": 33,
                            "daylightName": "(UTC-05:00) Eastern Time (US & Canada)",
                            "daylightDate": "yyyy-mm-ddThh:mm:ss,day_of_week",
                            "daylightBias": 33
                        }
                    }
                }
             */

            string timeInfoJson = await GetPropertyAsync(DeviceManagementClient.ReportedTimeInfoProperty);

            JsonReader jsonReader = new JsonReader(timeInfoJson);

            TimeInfo timeInfo = new TimeInfo();
            jsonReader.GetDate("timeInfo.localTime", out timeInfo.localTime);
            jsonReader.GetString("timeInfo.ntpServer", out timeInfo.ntpServer);
            jsonReader.GetLong("timeInfo.timeZone.bias", out timeInfo.timeZoneInformation.bias);
            jsonReader.GetString("timeInfo.timeZone.standardName", out timeInfo.timeZoneInformation.standardName);
            jsonReader.GetDate("timeInfo.timeZone.standardDate", out timeInfo.timeZoneInformation.standardDate);
            jsonReader.GetLong("timeInfo.timeZone.standardBias", out timeInfo.timeZoneInformation.standardBias);
            jsonReader.GetString("timeInfo.timeZone.daylightName", out timeInfo.timeZoneInformation.daylightName);
            jsonReader.GetDate("timeInfo.timeZone.daylightDate", out timeInfo.timeZoneInformation.daylightDate);
            jsonReader.GetLong("timeInfo.timeZone.daylightBias", out timeInfo.timeZoneInformation.daylightBias);

            return timeInfo;
        }

        public async Task<DeviceStatus> GetDeviceStatusAsync()
        {
            string deviceStatusJson = await GetPropertyAsync(DeviceManagementClient.ReportedDeviceStatusProperty);

            JsonReader jsonReader = new JsonReader(deviceStatusJson);

            DeviceStatus deviceStatus = new DeviceStatus();

            // set the different fields.
            jsonReader.GetLong("deviceStatus.secureBootState", out deviceStatus.secureBootState);
            jsonReader.GetString("deviceStatus.macIpAddressV4", out deviceStatus.macAddressIpV4);
            jsonReader.GetString("deviceStatus.macIpAddressV6", out deviceStatus.macAddressIpV6);
            jsonReader.GetBool("deviceStatus.macAddressIsConnected", out deviceStatus.macAddressIsConnected);
            jsonReader.GetLong("deviceStatus.macAddressType", out deviceStatus.macAddressType);
            jsonReader.GetString("deviceStatus.osType", out deviceStatus.osType);
            jsonReader.GetLong("deviceStatus.batteryStatus", out deviceStatus.batteryStatus);
            jsonReader.GetLong("deviceStatus.batteryRemaining", out deviceStatus.batteryRemaining);
            jsonReader.GetLong("deviceStatus.batteryRuntime", out deviceStatus.batteryRuntime);

            return deviceStatus;
        }

        public void OnDesiredPropertiesChanged(DeviceTwinUpdateState updateState, string desiredPropertiesString)
        {
            // Loop and apply the new values...
            JsonReader jsonReader = new JsonReader(desiredPropertiesString);
            foreach (KeyValuePair<string, JProperty> pair in jsonReader.Properties)
            {
                Debug.WriteLine("------------------------------------------------------");
                string key = pair.Key;
                string value = pair.Value.ToString();
                if (this.supportedProperties.ContainsKey(pair.Key))
                {
                    Debug.WriteLine("Supported: " + key + "," + value);
                    SetPropertyAsync(pair.Key, pair.Value.ToString());
                }
                else
                {
                    Debug.WriteLine("Not Supported: " + key + "," + value);
                }
            }
        }

        //
        // Commands:
        //

        // This command initiates factory reset of the device
        public async Task StartFactoryReset()
        {
            var request = new DMMessage(DMCommand.FactoryReset);

            // Here we might want to set some reported properties:
            // ReportProperties("We're about to start factory reset... If you don't hear from me again, I'm dead");

            var result = await SystemConfiguratorProxy.SendCommandAsync(request);
            if (result.Context != 0)
            {
                throw new Exception();
            }
        }

        public async Task StartSystemReboot()
        {
            SystemRebootRequestResponse rebootAllowed = await requestHandler.IsSystemRebootAllowed();
            if (rebootAllowed != SystemRebootRequestResponse.StartNow)
            {
                // ToDo: What should happen if the the user blocks the restart?
                //       We need to have a policy on when to ask again.
                return;
            }

            var request = new DMMessage(DMCommand.RebootSystem);
            var result = await SystemConfiguratorProxy.SendCommandAsync(request);
            if (result.Context != 0)
            {
                throw new Exception();
            }
        }

        // This command checks if updates are available. 
        // TODO: work out complete protocol (find updates, apply updates etc.)
        public async Task<bool> CheckForUpdatesAsync()
        {
            var request = new DMMessage(DMCommand.CheckUpdates);
            var response = await SystemConfiguratorProxy.SendCommandAsync(request);
            return response.Context == 1;    // 1 means "updates available"
        }

        public async Task<Dictionary<string, AppInfo>> ListAppsAsync()
        {
            var request = new DMMessage(DMCommand.ListApps);
            var result = await SystemConfiguratorProxy.SendCommandAsync(request);
            var json = result.GetDataString();
            return AppInfo.SetOfAppsFromJson(json);
        }

        public async Task InstallAppAsync(AppxInstallInfo appxInstallInfo)
        {
            var request = new DMMessage(DMCommand.InstallApp);
            request.SetData(JsonConvert.SerializeObject(appxInstallInfo));

            var result = await SystemConfiguratorProxy.SendCommandAsync(request);
            if (result.Context != 0)
            {
                throw new Exception();
            }
        }

        public async Task UninstallAppAsync(AppxUninstallInfo appxUninstallInfo)
        {
            var request = new DMMessage(DMCommand.UninstallApp);
            request.SetData(JsonConvert.SerializeObject(appxUninstallInfo));

            var result = await SystemConfiguratorProxy.SendCommandAsync(request);
            if (result.Context != 0)
            {
                throw new Exception();
            }
        }

        public async Task<string> GetStartupForegroundAppAsync()
        {
            var request = new DMMessage(DMCommand.GetStartupForegroundApp);
            var result = await SystemConfiguratorProxy.SendCommandAsync(request);
            return result.GetDataString();
        }

        public async Task<List<string>> ListStartupBackgroundAppsAsync()
        {
            var request = new DMMessage(DMCommand.ListStartupBackgroundApps);
            var result = await SystemConfiguratorProxy.SendCommandAsync(request);
            return JsonConvert.DeserializeObject<List<string>>(result.GetDataString());
        }

        public async Task AddStartupAppAsync(StartupAppInfo startupAppInfo)
        {
            var request = new DMMessage(DMCommand.AddStartupApp);
            request.SetData(JsonConvert.SerializeObject(startupAppInfo));

            var result = await SystemConfiguratorProxy.SendCommandAsync(request);
            if (result.Context != 0)
            {
                throw new Exception();
            }
        }

        public async Task RemoveStartupAppAsync(StartupAppInfo startupAppInfo)
        {
            var request = new DMMessage(DMCommand.RemoveStartupApp);
            request.SetData(JsonConvert.SerializeObject(startupAppInfo));

            var result = await SystemConfiguratorProxy.SendCommandAsync(request);
            if (result.Context != 0)
            {
                throw new Exception();
            }
        }

        //
        // Private utilities
        //

        // Report property to DT
        private void ReportProperties(string allJson)
        {
            deviceTwin.ReportProperties(allJson);
        }

        private async Task<DMMethodResult> HandleRebootAsync(string request)
        {
            DMMethodResult result = new DMMethodResult();

            try
            {
                await StartSystemReboot();
                result.returnCode = 1;  // success
            }
            catch (Exception)
            {
                // returnCode is already set to 0 to indicate failure.
            }
            return result;
        }

        private async Task<DMMethodResult> HandleFactoryResetAsync(string request)
        {
            DMMethodResult result = new DMMethodResult();

            try
            {
                await StartFactoryReset();
                result.returnCode = 1;  // success
            }
            catch (Exception)
            {
                // returnCode is already set to 0 to indicate failure.
            }
            return result;
        }

        // Data members
        IDeviceManagementRequestHandler requestHandler;
        IDeviceTwin deviceTwin;
        Dictionary<string, Func<string, Task<DMMethodResult>>> supportedMethods;
        Dictionary<string, DMCommand> supportedProperties;
    }

}
