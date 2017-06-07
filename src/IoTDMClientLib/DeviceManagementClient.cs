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
using Microsoft.Azure.Devices.Shared;
using Microsoft.Devices.Management.Message;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using Windows.Foundation;
using Windows.Services.Store;
using Windows.Storage;

namespace Microsoft.Devices.Management
{
    // This is the main entry point into DM
    public class DeviceManagementClient
    {
        public IDeviceTwin DeviceTwin { get { return _deviceTwin; } }

        // Types
        public struct DMMethodResult
        {
            public uint returnCode;
            public string response;
        }

        public struct ExternalStorage
        {
            public string connectionString;
            public string containerName;
        }

        public struct GetCertificateDetailsParams
        {
            public string path;
            public string hash;
            public string connectionString;
            public string containerName;
            public string blobName;
        }

        class AppLifeCycleParameters
        {
            public string pkgFamilyName;
            public string action;
        }

        class StartupApps
        {
            public string foreground;
        }

        private class HandlerCallback : IClientHandlerCallBack
        {
            public HandlerCallback(IDeviceTwin deviceTwin)
            {
                _deviceTwin = deviceTwin;
            }

            public async Task ReportPropertiesAsync(string propertyName, JObject properties)
            {
                try
                {
                    JObject managementObj = new JObject();
                    managementObj[propertyName] = properties;

                    Dictionary<string, object> collection = new Dictionary<string, object>();
                    collection["microsoft"] = new
                    {
                        management = managementObj
                    };

                    await _deviceTwin.ReportProperties(collection);
                }
                catch (Exception e)
                {
                    Debug.WriteLine($"Failed to report property {propertyName} : \n {e}");
                }
            }

            public async Task SendMessageAsync(string message, IDictionary<string, string> properties)
            {
                await _deviceTwin.SendMessageAsync(message, properties);
            }

            private IDeviceTwin _deviceTwin;
        }

        private DeviceManagementClient(IDeviceTwin deviceTwin, IDeviceManagementRequestHandler requestHandler, ISystemConfiguratorProxy systemConfiguratorProxy)
        {
            this._deviceTwin = deviceTwin;
            this._requestHandler = requestHandler;
            this._systemConfiguratorProxy = systemConfiguratorProxy;
            this._externalStorage = new ExternalStorage();
            this._desiredPropertyMap = new Dictionary<string, IClientPropertyHandler>();
        }

        private void AddPropertyHandler(IClientPropertyHandler handler)
        {
            this._desiredPropertyMap.Add(handler.PropertySectionName, handler);
        }

        private async Task AddDirectMethodHandlerAsync(IClientDirectMethodHandler handler)
        {
            foreach (var pair in handler.GetDirectMethodHandler())
            {
                var guard = new DirectMethodGuard(pair.Key, pair.Value);
                await this._deviceTwin.SetMethodHandlerAsync(pair.Key, guard.Invoke);
            }
        }

        public static async Task<DeviceManagementClient> CreateAsync(IDeviceTwin deviceTwin, IDeviceManagementRequestHandler requestHandler)
        {
            var systemConfiguratorProxy = new SystemConfiguratorProxy();
            var clientCallback = new HandlerCallback(deviceTwin);
            DeviceManagementClient deviceManagementClient = Create(deviceTwin, requestHandler, systemConfiguratorProxy);
            await deviceTwin.SetMethodHandlerAsync("microsoft.management.immediateReboot", deviceManagementClient.ImmediateRebootMethodHandlerAsync);
            await deviceTwin.SetMethodHandlerAsync("microsoft.management.reportAllDeviceProperties", deviceManagementClient.ReportAllDevicePropertiesMethodHandler);
            await deviceTwin.SetMethodHandlerAsync("microsoft.management.startAppSelfUpdate", deviceManagementClient.StartAppSelfUpdateMethodHandlerAsync);
            await deviceTwin.SetMethodHandlerAsync("microsoft.management.getCertificateDetails", deviceManagementClient.GetCertificateDetailsHandlerAsync);
            await deviceTwin.SetMethodHandlerAsync("microsoft.management.factoryReset", deviceManagementClient.FactoryResetHandlerAsync);
            await deviceTwin.SetMethodHandlerAsync("microsoft.management.manageAppLifeCycle", deviceManagementClient.ManageAppLifeCycleHandlerAsync);

            var deviceHealthAttestationHandler = new DeviceHealthAttestationHandler(clientCallback, systemConfiguratorProxy);
            deviceManagementClient.AddPropertyHandler(deviceHealthAttestationHandler);
            await deviceManagementClient.AddDirectMethodHandlerAsync(deviceHealthAttestationHandler);

            return deviceManagementClient;
        }

        internal static DeviceManagementClient Create(IDeviceTwin deviceTwin, IDeviceManagementRequestHandler requestHandler, ISystemConfiguratorProxy systemConfiguratorProxy)
        {
            return new DeviceManagementClient(deviceTwin, requestHandler, systemConfiguratorProxy);
        }

        public async Task ApplyDesiredStateAsync()
        {
            Debug.WriteLine("Applying desired state...");

            Dictionary<string, object> desiredProperties = await this._deviceTwin.GetDesiredPropertiesAsync();
            object node = null;
            if (desiredProperties.TryGetValue("microsoft", out node) && node != null && node is JObject)
            {
                JObject microsoftNode = (JObject)node;
                JToken token = microsoftNode.GetValue("management");
                if (token != null && token is JObject)
                {
                    // We won't await on this call to let it happen in the background...
                    ApplyDesiredStateAsync((JObject)token);
                }
            }
        }

        public void ApplyDesiredStateAsync(TwinCollection desiredProperties)
        {
            Debug.WriteLine("Applying desired state...");

            try
            {
                JObject dmNode = (JObject)desiredProperties["microsoft"]["management"];
                ApplyDesiredStateAsync(dmNode);
            }
            catch (Exception)
            {
                Debug.WriteLine("No properties.desired.microsoft.management is found.");
            }
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
        }

        internal async Task<IDictionary<string, Message.AppInfo>> ListAppsAsync()
        {
            var request = new Message.ListAppsRequest();
            var result = await this._systemConfiguratorProxy.SendCommandAsync(request);
            return (result as Message.ListAppsResponse).Apps;
        }

        internal async Task<Message.AppInstallResponse> InstallAppAsync(Message.AppInstallRequestData requestData)
        {
            Debug.WriteLine("Installing: " + requestData.PackageFamilyName);

            var request = new Message.AppInstallRequest(requestData);
            return await this._systemConfiguratorProxy.SendCommandAsync(request) as Message.AppInstallResponse;
        }

        internal async Task<Message.AppUninstallResponse> UninstallAppAsync(Message.AppUninstallRequestData requestData)
        {
            Debug.WriteLine("Uninstalling: " + requestData.PackageFamilyName);

            var request = new Message.AppUninstallRequest(requestData);
            return await this._systemConfiguratorProxy.SendCommandAsync(request) as Message.AppUninstallResponse;
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
            await this._systemConfiguratorProxy.SendCommandAsync(request);
        }

        internal async Task RemoveStartupAppAsync(Message.StartupAppInfo startupAppInfo)
        {
            var request = new Message.RemoveStartupAppRequest(startupAppInfo);
            await this._systemConfiguratorProxy.SendCommandAsync(request);
        }

        internal async Task AppLifecycleAsync(Message.AppLifecycleInfo appInfo)
        {
            var request = new Message.AppLifecycleRequest(appInfo);
            await this._systemConfiguratorProxy.SendCommandAsync(request);
        }

        public enum RebootRequestStatus
        {
            Allowed,
            Disabled,
            InActiveHours,
            RejectedByApp
        }

        private static string RebootRequestStatusString(RebootRequestStatus status)
        {
            switch (status)
            {
                case RebootRequestStatus.Allowed: return "allowed";
                case RebootRequestStatus.Disabled: return "disabled";
                case RebootRequestStatus.InActiveHours: return "inActiveHours";
                case RebootRequestStatus.RejectedByApp: return "rejectedByApp";
            }
            return status.ToString();
        }

        private void ReportImmediateRebootStatus(RebootRequestStatus rebootRequestStatus, string rebootCmdTime)
        {
            Dictionary<string, object> collection = new Dictionary<string, object>();
            collection["microsoft"] = new
            {
                management = new
                {
                    rebootInfo = new
                    {
                        lastRebootCmdTime = rebootCmdTime,
                        lastRebootCmdStatus = (rebootRequestStatus == RebootRequestStatus.Allowed ? "accepted" : RebootRequestStatusString(rebootRequestStatus))
                    }
                }
            };

            _deviceTwin.ReportProperties(collection);
        }

        public async Task<RebootRequestStatus> IsRebootAllowedBySystem()
        {
            var request = new Message.GetWindowsUpdateRebootPolicyRequest();
            var response = await this._systemConfiguratorProxy.SendCommandAsync(request) as Message.GetWindowsUpdateRebootPolicyResponse;
            if (!response.configuration.allow)
            {
                return RebootRequestStatus.Disabled;
            }

            Message.GetWindowsUpdatePolicyResponse updatePolicy = await this.GetWindowsUpdatePolicyAsync();
            uint nowHour = (uint)DateTime.Now.Hour;
            if (updatePolicy.configuration.activeHoursStart <= nowHour && nowHour < updatePolicy.configuration.activeHoursEnd)
            {
                return RebootRequestStatus.InActiveHours;
            }

            return RebootRequestStatus.Allowed;
        }

        private async Task<RebootRequestStatus> IsRebootAllowedByApp()
        {
            bool allowed = await this._requestHandler.IsSystemRebootAllowed();
            if (!allowed)
            {
                return RebootRequestStatus.RejectedByApp;
            }

            return RebootRequestStatus.Allowed;
        }

        private async Task<string> ImmediateRebootMethodHandlerAsync(string jsonParam)
        {
            string rebootCmdTime = DateTime.Now.ToString("yyyy-MM-ddTHH:mm:ssZ");

            RebootRequestStatus rebootRequestStatus = await IsRebootAllowedBySystem();
            if (rebootRequestStatus != RebootRequestStatus.Allowed)
            {
                // Report to the device twin
                ReportImmediateRebootStatus(rebootRequestStatus, rebootCmdTime);

                // Return details in the method return payload
                return JsonConvert.SerializeObject(new { response = RebootRequestStatusString(rebootRequestStatus) });
            }

            // Start the reboot operation asynchrnously, which may or may not succeed
            this.ImmediateRebootAsync(rebootCmdTime);
            return JsonConvert.SerializeObject(new { response = "accepted" });
        }

        public async Task ImmediateRebootAsync()
        {
            await ImmediateRebootAsync(DateTime.Now.ToString("yyyy-MM-ddTHH:mm:ssZ"));
        }

        public async Task ImmediateRebootAsync(string rebootCmdTime)
        {
            RebootRequestStatus rebootRequestStatus = await IsRebootAllowedByApp();

            // Report to the device twin
            ReportImmediateRebootStatus(rebootRequestStatus, rebootCmdTime);

            if (rebootRequestStatus != RebootRequestStatus.Allowed)
            {
                return;
            }

            var request = new Message.ImmediateRebootRequest();
            request.lastRebootCmdTime = rebootCmdTime;
            await this._systemConfiguratorProxy.SendCommandAsync(request);
        }

        private void ReportSelfUpdateStatus(string lastCheckValue, string statusValue)
        {
            Dictionary<string, object> collection = new Dictionary<string, object>();
            collection["microsoft"] = new
            {
                management = new
                {
                    appUpdate = new
                    {
                        lastCheck = lastCheckValue,
                        status = statusValue,
                    }
                }
            };
            _deviceTwin.ReportProperties(collection);
        }

        private async Task StartAppSelfUpdate()
        {
            Debug.WriteLine("Check for updates...");
            StoreContext context = StoreContext.GetDefault();

            // Check for updates...
            string lastCheck = DateTime.Now.ToString("yyyy-MM-ddTHH:mm:ssZ");

            ReportSelfUpdateStatus(lastCheck, "checkStarting");

            IReadOnlyList<StorePackageUpdate> updates = await context.GetAppAndOptionalStorePackageUpdatesAsync();
            if (updates.Count == 0)
            {
                ReportSelfUpdateStatus(lastCheck, "noUpdates");
                return;
            }

            // Download and install the updates...
            IAsyncOperationWithProgress<StorePackageUpdateResult, StorePackageUpdateStatus> downloadOperation =
                context.RequestDownloadAndInstallStorePackageUpdatesAsync(updates);

            ReportSelfUpdateStatus(lastCheck, "updatesDownloadingAndInstalling");

            // Wait for completion...
            StorePackageUpdateResult result = await downloadOperation.AsTask();

            ReportSelfUpdateStatus(lastCheck, result.OverallState == StorePackageUpdateState.Completed ? "installed" : "failed");

            return;
        }

        private Task<string> StartAppSelfUpdateMethodHandlerAsync(string jsonParam)
        {
            Debug.WriteLine("StartAppSelfUpdateMethodHandlerAsync");

            StartAppSelfUpdate();

            return Task.FromResult(JsonConvert.SerializeObject(new { response = "succeeded" }));
        }

        private async Task GetCertificateDetailsAsync(string jsonParam)
        {
            GetCertificateDetailsParams parameters = JsonConvert.DeserializeObject<GetCertificateDetailsParams>(jsonParam);

            var request = new Message.GetCertificateDetailsRequest();
            request.path = parameters.path;
            request.hash = parameters.hash;

            Message.GetCertificateDetailsResponse response = await _systemConfiguratorProxy.SendCommandAsync(request) as Message.GetCertificateDetailsResponse;

            string jsonString = JsonConvert.SerializeObject(response);
            Debug.WriteLine("response = " + jsonString);

            var info = new Message.AzureFileTransferInfo()
            {
                ConnectionString = parameters.connectionString,
                ContainerName = parameters.containerName,
                BlobName = parameters.blobName,
                Upload = true,
                LocalPath = ""
            };

            var appLocalDataFile = await ApplicationData.Current.TemporaryFolder.CreateFileAsync(parameters.blobName, CreationCollisionOption.ReplaceExisting);
            using (StreamWriter writer = new StreamWriter(await appLocalDataFile.OpenStreamForWriteAsync()))
            {
                await writer.WriteAsync(jsonString);
            }
            await IoTDMClient.AzureBlobFileTransfer.UploadFile(info, appLocalDataFile);

            await appLocalDataFile.DeleteAsync();
        }

        private Task<string> GetCertificateDetailsHandlerAsync(string jsonParam)
        {
            Debug.WriteLine("GetCertificateDetailsHandlerAsync");

            var response = new { response = "succeeded", reason = "" };
            try
            {
                // Submit the work and return immediately.
                GetCertificateDetailsAsync(jsonParam);
            }
            catch(Exception e)
            {
                response = new { response = "rejected:", reason = e.Message };
            }

            return Task.FromResult(JsonConvert.SerializeObject(response));
        }

        private async Task FactoryResetAsync(Message.FactoryResetRequest request)
        {
            await _systemConfiguratorProxy.SendCommandAsync(request);
        }

        private async Task FactoryResetAsync(string jsonParam)
        {
            await FactoryResetAsync(JsonConvert.DeserializeObject<Message.FactoryResetRequest>(jsonParam));
        }

        public async Task FactoryResetAsync(bool clearTPM, string recoveryPartitionGUID)
        {
            var request = new Message.FactoryResetRequest();
            request.clearTPM = clearTPM;
            request.recoveryPartitionGUID = recoveryPartitionGUID;
            await FactoryResetAsync(request);
        }

        private Task<string> FactoryResetHandlerAsync(string jsonParam)
        {
            Debug.WriteLine("FactoryResetHandlerAsync");

            var response = new { response = "succeeded", reason = "" };
            try
            {
                // Submit the work and return immediately.
                FactoryResetAsync(jsonParam);
            }
            catch (Exception e)
            {
                response = new { response = "rejected:", reason = e.Message };
            }

            return Task.FromResult(JsonConvert.SerializeObject(response));
        }

        private Task<string> ManageAppLifeCycleHandlerAsync(string jsonParam)
        {
            Debug.WriteLine("ManageAppLifeCycleHandlerAsync");

            var response = new { response = "succeeded", reason = "" };
            try
            {
                AppLifeCycleParameters appLifeCycleParameters = JsonConvert.DeserializeObject<AppLifeCycleParameters>(jsonParam);

                Message.AppLifecycleInfo appLifeCycleInfo = new Message.AppLifecycleInfo();
                appLifeCycleInfo.AppId = appLifeCycleParameters.pkgFamilyName;
                appLifeCycleInfo.Start = appLifeCycleParameters.action == "start";
                var request = new Message.AppLifecycleRequest(appLifeCycleInfo);
                _systemConfiguratorProxy.SendCommandAsync(request);
            }
            catch (Exception e)
            {
                response = new { response = "rejected:", reason = e.Message };
            }

            return Task.FromResult(JsonConvert.SerializeObject(response));
        }

        private static async void ProcessDesiredCertificateConfiguration(
            DeviceManagementClient client,
            string connectionString,
            string containerName,
            Message.CertificateConfiguration certificateConfiguration)
        {

            await IoTDMClient.CertificateManagement.DownloadCertificates(client, connectionString, containerName, certificateConfiguration);
            var request = new Message.SetCertificateConfigurationRequest(certificateConfiguration);
            client._systemConfiguratorProxy.SendCommandAsync(request);
        }
        private class WifiProfileConfigurationComparer : IEqualityComparer<WifiProfileConfiguration>
        {
            public bool Equals(WifiProfileConfiguration x, WifiProfileConfiguration y)
            {
                return (x.Name == y.Name);
            }
            public int GetHashCode(WifiProfileConfiguration x)
            {
                return x.Name.GetHashCode();
            }

        }
        private async void ProcessDesiredWifiConfiguration(
            DeviceManagementClient client,
            string connectionString,
            string containerName,
            Message.WifiConfiguration desiredConfiguration)
        {
            // Get installed wifi profiles
            var getInstalledResponse = await client.GetWifiConfigurationAsync();
            var reportedConfigurationProfiles = getInstalledResponse.Configuration.Profiles;
            var desiredConfigurationProfiles = desiredConfiguration.Profiles;

            // Only profiles that don't already exist need to be installed
            var needToAdd = desiredConfigurationProfiles.Where((config) => { return !config.Uninstall; }).Except(reportedConfigurationProfiles, new WifiProfileConfigurationComparer());
            // Only profiles that already exist need to be uninstalled
            var needToRemove = desiredConfigurationProfiles.Where((config) => { return config.Uninstall; }).Intersect(reportedConfigurationProfiles, new WifiProfileConfigurationComparer());
            // Create list of profiles for SystemConfigurator based on what NEEDS to be done
            var adjustedConfig = new WifiConfiguration() { Applying = desiredConfiguration.Applying, Reporting = desiredConfiguration.Reporting };
            adjustedConfig.Profiles = needToRemove.Union(needToAdd).ToList();

            // Download profiles needed for adding from Azure and load XML into WifiProfileConfiguration.Xml
            await WifiManagement.UpdateConfigWithProfileXmlAsync(client, connectionString, needToAdd);

            // Let SystemConfigurator do the actual work
            var request = new Message.SetWifiConfigurationRequest(adjustedConfig);
            var response = await client._systemConfiguratorProxy.SendCommandAsync(request);

            if (response.Status == ResponseStatus.Success)
            {
                var configToUpdateTwin = await client.GetWifiConfigurationAsync();
                var profilesToReport = configToUpdateTwin.Configuration.Profiles;
                foreach (var removed in needToRemove)
                {
                    configToUpdateTwin.Configuration.Profiles.Add(removed);
                }

                var jsonToReport = configToUpdateTwin.Configuration.ToJson(true);
                var reportString = $"{{\n \"management\" : {{\n \"wifi\" : {jsonToReport.ToString()}\n }}\n }}\n";
                Debug.WriteLine("Report:\n" + reportString);

                Dictionary<string, object> collection = new Dictionary<string, object>();
                collection["microsoft"] = JsonConvert.DeserializeObject(reportString);
                await client.DeviceTwin.ReportProperties(collection);
            }
        }

        public async Task AllowReboots(bool allowReboots)
        {
            var configuration = new WindowsUpdateRebootPolicyConfiguration();
            configuration.allow = allowReboots;
            IResponse response = await this._systemConfiguratorProxy.SendCommandAsync(new SetWindowsUpdateRebootPolicyRequest(configuration));
            if (response.Status != ResponseStatus.Success)
            {
                throw new Exception("Error: failed to set update reboot policy.");
            }
        }

        private async void ApplyDesiredTimeSettings(JToken jsonValue)
        {
            // Default JsonConvert Deserializing changes ISO8601 date fields to "mm/dd/yyyy hh:mm:ss".
            // We need to preserve the ISO8601 since that's the format SystemConfigurator understands.
            // Because of that, we are not using:
            // Message.SetTimeInfo requestInfo = JsonConvert.DeserializeObject<Message.SetTimeInfo>(fieldsJson);

            Message.SetTimeInfoRequestData data = new Message.SetTimeInfoRequestData();

            JObject subProperties = (JObject)jsonValue;
            data.ntpServer = (string)subProperties.Property("ntpServer").Value;
            data.timeZoneBias = (int)subProperties.Property("timeZoneBias").Value;

            data.timeZoneStandardBias = (int)subProperties.Property("timeZoneStandardBias").Value;
            string standardDateString = subProperties.Property("timeZoneStandardDate").Value.ToString();
            DateTime standardDate = DateTime.Parse(standardDateString).ToUniversalTime();
            data.timeZoneStandardDate = standardDate.ToString("yyyy-MM-ddTHH:mm:ssZ");
            data.timeZoneStandardName = (string)subProperties.Property("timeZoneStandardName").Value;
            data.timeZoneStandardDayOfWeek = (int)subProperties.Property("timeZoneStandardDayOfWeek").Value;

            data.timeZoneDaylightBias = (int)subProperties.Property("timeZoneDaylightBias").Value;
            string daylightDateString = subProperties.Property("timeZoneDaylightDate").Value.ToString();
            DateTime daylightDate = DateTime.Parse(daylightDateString).ToUniversalTime();
            data.timeZoneDaylightDate = daylightDate.ToString("yyyy-MM-ddTHH:mm:ssZ");
            data.timeZoneDaylightName = (string)subProperties.Property("timeZoneDaylightName").Value;
            data.timeZoneDaylightDayOfWeek = (int)subProperties.Property("timeZoneDaylightDayOfWeek").Value;

            Message.SetTimeInfoRequest request = new Message.SetTimeInfoRequest(data);

            await this._systemConfiguratorProxy.SendCommandAsync(request);

            await ReportTimeInfoAsync();
        }

        public void ApplyDesiredStateAsync(JObject dmNode)
        {
            // ToDo: We should not throw here. All problems need to be logged.
            Message.CertificateConfiguration certificateConfiguration = null;
            Message.WifiConfiguration wifiConfiguration = null;
            JObject appsConfiguration = null;

            foreach (var managementProperty in dmNode.Children().OfType<JProperty>())
            {
                switch (managementProperty.Name)
                {
                    case "scheduledReboot":
                        {
                            Debug.WriteLine("scheduledReboot = " + managementProperty.Value.ToString());

                            JObject subProperties = (JObject)managementProperty.Value;

                            var request = new Message.SetRebootInfoRequest();

                            DateTime singleRebootTime = DateTime.Parse(subProperties.Property("singleRebootTime").Value.ToString());
                            request.singleRebootTime = singleRebootTime.ToString("yyyy-MM-ddTHH:mm:ssZ");

                            DateTime dailyRebootTime = DateTime.Parse(subProperties.Property("dailyRebootTime").Value.ToString());
                            request.dailyRebootTime = dailyRebootTime.ToString("yyyy-MM-ddTHH:mm:ssZ");

                            this._systemConfiguratorProxy.SendCommandAsync(request);
                        }
                        break;
                    case "externalStorage":
                        {
                            Debug.WriteLine("externalStorage = " + managementProperty.Value.ToString());

                            JObject subProperties = (JObject)managementProperty.Value;

                            _externalStorage.connectionString = (string)subProperties.Property("connectionString").Value;
                            _externalStorage.containerName = (string)subProperties.Property("container").Value;
                        }
                        break;
                    case "certificates":
                        {
                            // Capture the configuration here.
                            // To apply the configuration we need to wait until externalStorage has been configured too.
                            Debug.WriteLine("CertificateConfiguration = " + managementProperty.Value.ToString());
                            certificateConfiguration = JsonConvert.DeserializeObject<CertificateConfiguration>(managementProperty.Value.ToString());
                        }
                        break;
                    case "timeInfo":
                        {
                            Debug.WriteLine("timeInfo = " + managementProperty.Value.ToString());
                            ApplyDesiredTimeSettings(managementProperty.Value);
                        }
                        break;
                    case "wifi":
                        // Capture the configuration here.
                        // To apply the configuration we need to wait until externalStorage has been configured too.
                        var valueString = managementProperty.Value.ToString();
                        Debug.WriteLine("WifiConfiguration = " + valueString);
                        wifiConfiguration = WifiConfiguration.Parse(valueString);
                        break;
                    case "windowsUpdatePolicy":
                        {
                            Debug.WriteLine("windowsUpdatePolicy = " + managementProperty.Value.ToString());
                            var configuration = JsonConvert.DeserializeObject<WindowsUpdatePolicyConfiguration>(managementProperty.Value.ToString());
                            this._systemConfiguratorProxy.SendCommandAsync(new SetWindowsUpdatePolicyRequest(configuration));
                        }
                        break;
                    case "windowsUpdates":
                        {
                            Debug.WriteLine("windowsUpdates = " + managementProperty.Value.ToString());
                            var configuration = JsonConvert.DeserializeObject<SetWindowsUpdatesConfiguration>(managementProperty.Value.ToString());
                            this._systemConfiguratorProxy.SendCommandAsync(new SetWindowsUpdatesRequest(configuration));
                        }
                        break;
                    case "apps":
                        {
                            Debug.WriteLine("apps = " + managementProperty.Value.ToString());
                            appsConfiguration = (JObject)managementProperty.Value;
                        }
                        break;
                    case "startupApps":
                        {
                            Debug.WriteLine("startupApps = " + managementProperty.Value.ToString());
                            var startupApps = JsonConvert.DeserializeObject<StartupApps>(managementProperty.Value.ToString());
                            StartupAppInfo foregroundApp = new StartupAppInfo(startupApps.foreground, false /*!background*/);
                            this._systemConfiguratorProxy.SendCommandAsync(new AddStartupAppRequest(foregroundApp));
                        }
                        break;
                    default:
                        // Not supported
                        break;
                }

                IClientPropertyHandler handler;
                if (this._desiredPropertyMap.TryGetValue(managementProperty.Name, out handler))
                {
                    try
                    {
                        Debug.WriteLine($"{managementProperty.Name} = {managementProperty.Value.ToString()}");
                        handler.OnDesiredPropertyChange((JObject)managementProperty.Value);
                    }
                    catch (Exception e)
                    {
                        Debug.WriteLine($"Exception caught while handling desired property - {managementProperty.Name}");
                        Debug.WriteLine(e);
                        throw;
                    }
                }
            }

            // Now, handle the operations that depend on others in the necessary order.
            // By now, Azure storage information should have been captured.

            if (!String.IsNullOrEmpty(_externalStorage.connectionString))
            {
                if (appsConfiguration != null)
                {
                    AppxManagement.ApplyDesiredAppsConfiguration(this, _externalStorage.connectionString, appsConfiguration);
                }

                // Some operations require the default container to be specified...
                if (!String.IsNullOrEmpty(_externalStorage.containerName))
                {
                    if (certificateConfiguration != null)
                    {
                        ProcessDesiredCertificateConfiguration(this, _externalStorage.connectionString, _externalStorage.containerName, certificateConfiguration);
                    }
                    if (wifiConfiguration != null)
                    {
                        ProcessDesiredWifiConfiguration(this, _externalStorage.connectionString, _externalStorage.containerName, wifiConfiguration);
                    }
                }
            }
        }

        private async Task<Message.GetTimeInfoResponse> GetTimeInfoAsync()
        {
            var request = new Message.GetTimeInfoRequest();
            return (await this._systemConfiguratorProxy.SendCommandAsync(request) as Message.GetTimeInfoResponse);
        }

        private async Task<Message.GetCertificateConfigurationResponse> GetCertificateConfigurationAsync()
        {
            var request = new Message.GetCertificateConfigurationRequest();
            return (await this._systemConfiguratorProxy.SendCommandAsync(request) as Message.GetCertificateConfigurationResponse);
        }

        private async Task<Message.GetRebootInfoResponse> GetRebootInfoAsync()
        {
            var request = new Message.GetRebootInfoRequest();
            return (await this._systemConfiguratorProxy.SendCommandAsync(request) as Message.GetRebootInfoResponse);
        }

        private async Task<Message.GetDeviceInfoResponse> GetDeviceInfoAsync()
        {
            var request = new Message.GetDeviceInfoRequest();
            return (await this._systemConfiguratorProxy.SendCommandAsync(request) as Message.GetDeviceInfoResponse);
        }

        private async Task<Message.GetWindowsUpdatePolicyResponse> GetWindowsUpdatePolicyAsync()
        {
            var request = new Message.GetWindowsUpdatePolicyRequest();
            return (await this._systemConfiguratorProxy.SendCommandAsync(request) as Message.GetWindowsUpdatePolicyResponse);
        }

        private async Task<Message.GetWindowsUpdatesResponse> GetWindowsUpdatesAsync()
        {
            var request = new Message.GetWindowsUpdatesRequest();
            return (await this._systemConfiguratorProxy.SendCommandAsync(request) as Message.GetWindowsUpdatesResponse);
        }

        private async Task<Message.GetWifiConfigurationResponse> GetWifiConfigurationAsync()
        {
            var request = new Message.GetWifiConfigurationRequest();
            var response = (await this._systemConfiguratorProxy.SendCommandAsync(request) as Message.GetWifiConfigurationResponse);

            if (Debugger.IsAttached)
            {
                foreach (var profile in response.Configuration.Profiles)
                {
                    Debug.WriteLine($"{profile.Name} : path={profile.Path} uninstall={profile.Uninstall}");
                }
            }

            return response;
        }

        private async Task ReportTimeInfoAsync()
        {
            Debug.WriteLine("Reporting timeInfo...");

            Message.GetTimeInfoResponse timeInfoResponse = await GetTimeInfoAsync();
            Dictionary<string, object> collection = new Dictionary<string, object>();
            collection["microsoft"] = new
            {
                management = new
                {
                    timeInfo = timeInfoResponse.data,
                }
            };

            _deviceTwin.ReportProperties(collection);
        }

        private async Task ReportAllDeviceProperties()
        {
            Debug.WriteLine("ReportAllDeviceProperties");
            Debug.WriteLine("Querying start: " + DateTime.Now.ToString());

            Message.GetTimeInfoResponse timeInfoResponse = await GetTimeInfoAsync();
            Message.GetCertificateConfigurationResponse certificateConfigurationResponse = await GetCertificateConfigurationAsync();
            Message.GetRebootInfoResponse rebootInfoResponse = await GetRebootInfoAsync();
            Message.GetDeviceInfoResponse deviceInfoResponse = await GetDeviceInfoAsync();
            Message.GetWindowsUpdatePolicyResponse windowsUpdatePolicyResponse = await GetWindowsUpdatePolicyAsync();
            Message.GetWindowsUpdatesResponse windowsUpdatesResponse = await GetWindowsUpdatesAsync();
            Message.GetWifiConfigurationResponse wifiResponse = await GetWifiConfigurationAsync();

            JObject managementObj = new JObject();
            managementObj["timeInfo"] = JObject.FromObject(timeInfoResponse.data);
            managementObj["certificates"] = JObject.FromObject(certificateConfigurationResponse);
            managementObj["rebootInfo"] = JObject.FromObject(rebootInfoResponse);
            managementObj["deviceInfo"] = JObject.FromObject(deviceInfoResponse);
            managementObj["windowsUpdatePolicy"] = JObject.FromObject(windowsUpdatePolicyResponse.configuration);
            managementObj["windowsUpdates"] = JObject.FromObject(windowsUpdatesResponse.configuration);
            managementObj["wifi"] = JObject.Parse(wifiResponse.Configuration.ToJson(true).ToString());

            foreach (var handler in this._desiredPropertyMap.Values)
            {
                managementObj[handler.PropertySectionName] = await handler.GetReportedPropertyAsync();
            }

            Debug.WriteLine("Querying end: " + DateTime.Now.ToString());

            Dictionary<string, object> collection = new Dictionary<string, object>();
            collection["microsoft"] = new
            {
                management = managementObj
            };

            Debug.WriteLine($"Report properties: {managementObj.ToString()}");
            _deviceTwin.ReportProperties(collection);
        }

        private async Task<string> ReportAllDevicePropertiesMethodHandler(string jsonParam)
        {
            Debug.WriteLine("ReportAllDevicePropertiesMethodHandler");

            ReportAllDeviceProperties();

            return JsonConvert.SerializeObject(new { response = "success" });
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
        ExternalStorage _externalStorage;
        Dictionary<string, IClientPropertyHandler> _desiredPropertyMap;
    }

}
