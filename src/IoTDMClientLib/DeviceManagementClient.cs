using Microsoft.Azure.Devices.Shared;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Threading.Tasks;

namespace Microsoft.Devices.Management
{
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

        private DeviceManagementClient(IDeviceTwin deviceTwin, IDeviceManagementRequestHandler requestHandler, ISystemConfiguratorProxy systemConfiguratorProxy)
        {
            this._deviceTwin = deviceTwin;
            this._requestHandler = requestHandler;
            this._systemConfiguratorProxy = systemConfiguratorProxy;
        }

        public static DeviceManagementClient Create(IDeviceTwin deviceTwin, IDeviceManagementRequestHandler requestHandler)
        {
            DeviceManagementClient deviceManagementClient = Create(deviceTwin, requestHandler, new SystemConfiguratorProxy());
            deviceTwin.SetMethodHandlerAsync("microsoft.management.immediateReboot", deviceManagementClient.ImmediateRebootMethodHandlerAsync);
            deviceTwin.SetMethodHandlerAsync("microsoft.management.appInstall", deviceManagementClient.AppInstallMethodHandlerAsync);
            return deviceManagementClient;
        }

        internal static DeviceManagementClient Create(IDeviceTwin deviceTwin, IDeviceManagementRequestHandler requestHandler, ISystemConfiguratorProxy systemConfiguratorProxy)
        {
            return new DeviceManagementClient(deviceTwin, requestHandler, systemConfiguratorProxy);
        }

        //
        // Commands:
        //

        // This command checks if updates are available. 
        // TODO: work out complete protocol (find updates, apply updates etc.)
        public async Task<bool> CheckForUpdatesAsync()
        {
            var request = new Message.CheckForUpdatesRequest();
            var response = await this._systemConfiguratorProxy.SendCommandAsync(request);
            return (response as Message.CheckForUpdatesResponse).UpdatesAvailable;
        }

        internal async Task TransferFileAsync(Message.AzureFileTransferInfo transferInfo)
        {
            // use C++ service to copy file to/from App LocalData
            var request = new Message.AzureFileTransferRequest(transferInfo);
            var result = await this._systemConfiguratorProxy.SendCommandAsync(request);
            if (result.Status != Message.ResponseStatus.Success)
            {
                throw new Exception();
            }
        }

        internal async Task<IDictionary<string, Message.AppInfo>> ListAppsAsync()
        {
            var request = new Message.ListAppsRequest();
            var result = await this._systemConfiguratorProxy.SendCommandAsync(request);
            return (result as Message.ListAppsResponse).Apps;
        }

        internal Task<string> AppInstallMethodHandlerAsync(string jsonParam)
        {
            try
            {
                // method should return immediately .. only validate the json param
                var appBlobInfo = JsonConvert.DeserializeObject<IoTDMClient.AppBlobInfo>(jsonParam);
                // task should run without blocking so resonse can be generated right away
                var appInstallTask = appBlobInfo.AppInstallAsync(this);
                // response with success
                var response = JsonConvert.SerializeObject(new { response = "accepted" });
                return Task.FromResult(response);
            }
            catch (Exception e)
            {
                // response with failure
                var response = JsonConvert.SerializeObject(new { response = "rejected", reason = e.Message });
                return Task.FromResult(response);
            }
        }

        internal async Task InstallAppAsync(Message.AppInstallInfo appInstallInfo)
        {
            var request = new Message.AppInstallRequest(appInstallInfo);
            var result = await this._systemConfiguratorProxy.SendCommandAsync(request);
            if (result.Status != Message.ResponseStatus.Success)
            {
                throw new Exception();
            }
        }

        internal async Task UninstallAppAsync(Message.AppUninstallInfo appUninstallInfo)
        {
            var request = new Message.AppUninstallRequest(appUninstallInfo);
            var result = await this._systemConfiguratorProxy.SendCommandAsync(request);
            if (result.Status != Message.ResponseStatus.Success)
            {
                throw new Exception();
            }
        }

        internal async Task<string> GetStartupForegroundAppAsync()
        {
            var request = new Message.GetStartupForegroundAppRequest();
            var result = await this._systemConfiguratorProxy.SendCommandAsync(request);
            return (result as Message.GetStartupForegroundAppResponse).StartupForegroundApp;
        }

        internal async Task<IList<string>> ListStartupBackgroundAppsAsync()
        {
            var request = new Message.ListStartupBackgroundAppsRequest();
            var result = await this._systemConfiguratorProxy.SendCommandAsync(request);
            return (result as Message.ListStartupBackgroundAppsResponse).StartupBackgroundApps;
        }

        internal async Task AddStartupAppAsync(Message.StartupAppInfo startupAppInfo)
        {
            var request = new Message.AddStartupAppRequest(startupAppInfo);
            var result = await this._systemConfiguratorProxy.SendCommandAsync(request);
            if (result.Status != Message.ResponseStatus.Success)
            {
                throw new Exception();
            }
        }

        internal async Task RemoveStartupAppAsync(Message.StartupAppInfo startupAppInfo)
        {
            var request = new Message.RemoveStartupAppRequest(startupAppInfo);
            var result = await this._systemConfiguratorProxy.SendCommandAsync(request);
            if (result.Status != Message.ResponseStatus.Success)
            {
                throw new Exception();
            }
        }

        internal async Task AppLifecycleAsync(Message.AppLifecycleInfo appInfo)
        {
            var request = new Message.AppLifecycleRequest(appInfo);
            var result = await this._systemConfiguratorProxy.SendCommandAsync(request);
            if (result.Status != Message.ResponseStatus.Success)
            {
                throw new Exception();
            }
        }

        private void ReportImmediateRebootStatus(bool rebootSuccessful)
        {
            Dictionary<string, object> collection = new Dictionary<string, object>();
            collection["microsoft"] = new
            {
                management = new
                {
                    lastRebootAttempt = new
                    {
                        time = DateTime.Now,
                        status = rebootSuccessful ? "success" : "failure"
                    }
                }
            };

            _deviceTwin.ReportProperties(collection);
        }

        private Task<string> ImmediateRebootMethodHandlerAsync(string jsonParam)
        {
            // Start the reboot operation asynchrnously, which may or may not succeed
            var rebootOp = this.ImmediateRebootAsync();

            // TODO: consult the active hours schedule to make sure reboot is allowed
            var rebootAllowed = true;

            var response = JsonConvert.SerializeObject(new { response = rebootAllowed ? "accepted" : "rejected" });

            return Task.FromResult(response);
        }

        public async Task ImmediateRebootAsync()
        {
            bool rebootSuccessful = (await this._requestHandler.IsSystemRebootAllowed() == SystemRebootRequestResponse.Accept);
            // Report status before actually initiating reboot, to avoid the race condition
            ReportImmediateRebootStatus(rebootSuccessful);
            if (rebootSuccessful)
            {
                var request = new Message.ImmediateRebootRequest();
                await this._systemConfiguratorProxy.SendCommandAsync(request);
            }
        }

        public async Task<DMMethodResult> DoFactoryResetAsync()
        {
            throw new NotImplementedException();
        }

        public void ProcessDeviceManagementProperties(TwinCollection desiredProperties)
        {
            foreach (KeyValuePair<string, object> dp in desiredProperties)
            {
                if (dp.Key == "microsoft" && dp.Value is JObject)
                {
                    JToken managementNode;
                    if ((dp.Value as JObject).TryGetValue("management", out managementNode))
                    {
                        foreach (var managementProperty in managementNode.Children().OfType<JProperty>())
                        {
                            switch (managementProperty.Name)
                            {
                                case "scheduledReboot":
                                    // TODO
                                    break;
                                default:
                                    // Not supported
                                    break;
                            }
                        }
                    }
                }
             }
        }

        public async Task<Message.TimeInfoResponse> GetTimeInfoAsync()
        {
            var request = new Message.TimeInfoRequest();
            return (await this._systemConfiguratorProxy.SendCommandAsync(request) as Message.TimeInfoResponse);
        }

        public async Task<RebootInfo> GetRebootInfoAsync()
        {
            string jsonString = await GetPropertyAsync(Message.DMMessageKind.GetRebootInfo);
            Debug.WriteLine(" json rebootInfo = " + jsonString);
            RebootInfoInternal rebootInfoInternal = JsonConvert.DeserializeObject<RebootInfoInternal>(jsonString);
            return new RebootInfo(rebootInfoInternal);
        }

        public async Task<DeviceStatus> GetDeviceStatusAsync()
        {
            string deviceStatusJson = await GetPropertyAsync(Message.DMMessageKind.GetDeviceStatus);
            Debug.WriteLine(" json deviceStatus = " + deviceStatusJson);
            return JsonConvert.DeserializeObject<DeviceStatus>(deviceStatusJson); ;
        }

        public async Task<DMMethodResult> ReportAllPropertiesAsync()
        {
            Debug.WriteLine("ReportAllPropertiesAsync()");
            DMMethodResult methodResult = new DMMethodResult();

            Dictionary<string, object> collection = new Dictionary<string, object>();
            collection["timeInfo"] = await GetTimeInfoAsync();
#if false // TODO
            collection["deviceStatus"] = await GetDeviceStatusAsync();
            collection["rebootInfo"] = await GetRebootInfoAsync();
#endif
            _deviceTwin.ReportProperties(collection);

            return methodResult;
        }

        //
        // Private utilities
        //

        private async Task SetPropertyAsync(Message.DMMessageKind command, string valueString)
        {
            throw new NotImplementedException();
        }

        private async Task<string> GetPropertyAsync(Message.DMMessageKind command)
        {
            throw new NotImplementedException();
        }

        // Data members
        ISystemConfiguratorProxy _systemConfiguratorProxy;
        IDeviceManagementRequestHandler _requestHandler;
        IDeviceTwin _deviceTwin;
    }

}
