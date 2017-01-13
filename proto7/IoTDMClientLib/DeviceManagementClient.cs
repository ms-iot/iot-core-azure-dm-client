using Microsoft.Azure.Devices.Shared;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading.Tasks;

namespace Microsoft.Devices.Management
{
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
        public string ToJson()
        {
            return JsonConvert.SerializeObject(this);
        }
    }

    public class AppxUninstallInfo
    {
        public string PackageFamilyName { get; set; }
        public bool StoreApp { get; set; }

        public string ToJson()
        {
            return JsonConvert.SerializeObject(this);
        }
    }

    public class TimeZoneInfo
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
        public DateTime localTime;
        public string ntpServer;
        public TimeZoneInfo timeZone;
    }

    public class RebootInfo
    {
        public DateTime lastRebootTime;
        public DateTime lastRebootCmdTime;
        public DateTime singleRebootTime;
        public DateTime dailyRebootTime;

        internal RebootInfo(RebootInfoInternal rebootInfoInternal)
        {
            if (!String.IsNullOrEmpty(rebootInfoInternal.lastRebootTime))
            {
                lastRebootTime = DateTime.Parse(rebootInfoInternal.lastRebootTime);
            }
            if (!String.IsNullOrEmpty(rebootInfoInternal.lastRebootCmdTime))
            {
                lastRebootCmdTime = DateTime.Parse(rebootInfoInternal.lastRebootCmdTime);
            }
            if (!String.IsNullOrEmpty(rebootInfoInternal.singleRebootTime))
            {
                singleRebootTime = DateTime.Parse(rebootInfoInternal.singleRebootTime);
            }
            if (!String.IsNullOrEmpty(rebootInfoInternal.dailyRebootTime))
            {
                dailyRebootTime = DateTime.Parse(rebootInfoInternal.dailyRebootTime);
            }
        }
    }

    internal class RebootInfoInternal
    {
        public string lastRebootTime;
        public string lastRebootCmdTime;
        public string singleRebootTime;
        public string dailyRebootTime;
    }

    // This is the main entry point into DM
    public class DeviceManagementClient
    {
        // Types
        public struct DMMethodResult
        {
            public uint returnCode;
            public string response;
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
            this._requestHandler = requestHandler;
            this._deviceTwin = deviceTwin;
        }

        public static DeviceManagementClient Create(IDeviceTwin _deviceTwin, IDeviceManagementRequestHandler requestHandler)
        {
            DeviceManagementClient deviceManagementClient = new DeviceManagementClient(_deviceTwin, requestHandler);
            _deviceTwin.SetManagementClient(deviceManagementClient);
            return deviceManagementClient;
        }

        public TwinCollection HandleDesiredPropertiesChanged(TwinCollection desiredProperties)
        {
            TwinCollection nonDMProperties = new TwinCollection();

            foreach (KeyValuePair<string, object> dp in desiredProperties)
            {
                string valueString = dp.Value.ToString();
                if (dp.Key == "timeInfo")
                {
                    if (!String.IsNullOrEmpty(valueString))
                    {
                        Debug.WriteLine(" timeInfo json = ", valueString);
                        SetPropertyAsync(DMCommand.SetTimeInfo, valueString);
                    }
                }
                else if (dp.Key == "rebootInfo")
                {
                    if (!String.IsNullOrEmpty(valueString))
                    {
                        Debug.WriteLine(" rebootInfo json = ", valueString);
                        SetPropertyAsync(DMCommand.SetRebootInfo, valueString);
                    }
                }
                else
                {
                    nonDMProperties[dp.Key] = dp.Value;
                }
            }

            return nonDMProperties;
        }

        //
        // Commands:
        //

        // This command checks if updates are available. 
        // TODO: work out complete protocol (find updates, apply updates etc.)
        public async Task<bool> CheckForUpdatesAsync()
        {
            var request = new DMMessage(DMCommand.CheckUpdates);
            var response = await SystemConfiguratorProxy.SendCommandAsync(request);
            return response.Context == 1;    // 1 means "updates available"
        }

        public async Task<Dictionary<string, AppInfo>> StartListApps()
        {
            var request = new DMMessage(DMCommand.ListApps);
            var result = await SystemConfiguratorProxy.SendCommandAsync(request);

            var json = result.GetDataString();
            return AppInfo.SetOfAppsFromJson(json);
        }

        public async Task<UInt32> StartInstallApp(AppxInstallInfo appxInstallInfo)
        {
            var request = new DMMessage(DMCommand.InstallApp);
            request.SetData(appxInstallInfo.ToJson());

            var result = await SystemConfiguratorProxy.SendCommandAsync(request);
            return result.Context;
        }

        public async Task<UInt32> StartUninstallApp(AppxUninstallInfo appxUninstallInfo)
        {
            var request = new DMMessage(DMCommand.UninstallApp);
            request.SetData(appxUninstallInfo.ToJson());

            var result = await SystemConfiguratorProxy.SendCommandAsync(request);
            return result.Context;
        }

        public async Task<DMMethodResult> RebootSystemAsync()
        {
            DMMethodResult methodResult = new DMMethodResult();

            try
            {
                SystemRebootRequestResponse rebootAllowed = await _requestHandler.IsSystemRebootAllowed();
                if (rebootAllowed != SystemRebootRequestResponse.StartNow)
                {
                    // ToDo: What should happen if the the user blocks the restart?
                    //       We need to have a policy on when to ask again.
                    methodResult.returnCode = (uint)DMStatus.Failure;
                    return methodResult;
                }

                var request = new DMMessage(DMCommand.RebootSystem);
                var result = await SystemConfiguratorProxy.SendCommandAsync(request);
                if (result.Context != 0)
                {
                    throw new Exception();
                }
                methodResult.returnCode = (uint)DMStatus.Success;
            }
            catch (Exception)
            {
                // returnCode is already set to 0 to indicate failure.
            }
            return methodResult;
        }

        public async Task<DMMethodResult> DoFactoryResetAsync()
        {
            DMMethodResult methodResult = new DMMethodResult();

            try
            {
                var request = new DMMessage(DMCommand.FactoryReset);

                // Here we might want to set some reported properties:
                // ReportProperties("We're about to start factory reset... If you don't hear from me again, I'm dead");

                var result = await SystemConfiguratorProxy.SendCommandAsync(request);
                if (result.Context != 0)
                {
                    throw new Exception();
                }
                methodResult.returnCode = (uint)DMStatus.Success;
            }
            catch (Exception)
            {
                // returnCode is already set to 0 to indicate failure.
            }
            return methodResult;
        }

        public async Task<TimeInfo> GetTimeInfoAsync()
        {
            string jsonString = await GetPropertyAsync(DMCommand.GetTimeInfo);
            Debug.WriteLine(" json timeInfo = " + jsonString);
            return JsonConvert.DeserializeObject<TimeInfo>(jsonString);
        }

        public async Task<RebootInfo> GetRebootInfoAsync()
        {
            string jsonString = await GetPropertyAsync(DMCommand.GetRebootInfo);
            Debug.WriteLine(" json rebootInfo = " + jsonString);
            RebootInfoInternal rebootInfoInternal = JsonConvert.DeserializeObject<RebootInfoInternal>(jsonString);
            return new RebootInfo(rebootInfoInternal);
        }

        public async Task<DeviceStatus> GetDeviceStatusAsync()
        {
            string deviceStatusJson = await GetPropertyAsync(DMCommand.GetDeviceStatus);
            Debug.WriteLine(" json deviceStatus = " + deviceStatusJson);
            return JsonConvert.DeserializeObject<DeviceStatus>(deviceStatusJson); ;
        }

        public async Task<DMMethodResult> ReportAllPropertiesAsync()
        {
            Debug.WriteLine("ReportAllPropertiesAsync()");
            DMMethodResult methodResult = new DMMethodResult();

            try
            {
                Azure.Devices.Shared.TwinCollection collection = new Azure.Devices.Shared.TwinCollection();
                collection["timeInfo"] = await GetTimeInfoAsync();
                collection["deviceStatus"] = await GetDeviceStatusAsync();
                collection["rebootInfo"] = await GetRebootInfoAsync();
                _deviceTwin.ReportProperties(collection);
                methodResult.returnCode = (uint)DMStatus.Success;
            }
            catch (Exception)
            {
                // returnCode is already set to 0 to indicate failure.
            }
            return methodResult;
        }

        //
        // Private utilities
        //

        private async Task SetPropertyAsync(DMCommand command, string valueString)
        {
            var request = new DMMessage(command);
            request.SetData(valueString);

            var result = await SystemConfiguratorProxy.SendCommandAsync(request);
            if (result.Context != (UInt32)DMStatus.Success)
            {
                throw new Exception();
            }
        }

        private async Task<string> GetPropertyAsync(DMCommand command)
        {
            var request = new DMMessage(command);
            var result = await SystemConfiguratorProxy.SendCommandAsync(request);
            if (result.Context != (UInt32)DMStatus.Success)
            {
                throw new Exception();
            }
            return result.GetDataString();
        }

        // Data members
        IDeviceManagementRequestHandler _requestHandler;
        IDeviceTwin _deviceTwin;
    }

}
