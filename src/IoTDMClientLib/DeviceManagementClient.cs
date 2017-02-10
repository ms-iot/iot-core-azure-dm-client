﻿using Microsoft.Azure.Devices.Shared;
using Microsoft.WindowsAzure.Storage;
using Microsoft.WindowsAzure.Storage.Blob;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System;
using System.Linq;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading.Tasks;
using Windows.Foundation;
using Windows.Services.Store;
using Windows.Storage;

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
            deviceTwin.SetMethodHandlerAsync("microsoft.management.reportAllProperties", deviceManagementClient.ReportAllMethodHandler);
            deviceTwin.SetMethodHandlerAsync("microsoft.management.appStartSelfUpdate", deviceManagementClient.AppStartSelfUpdateMethodHandlerAsync);
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

        public async Task<IDictionary<string, Message.AppInfo>> ListAppsAsync()
        {
            var request = new Message.ListAppsRequest();
            var result = await this._systemConfiguratorProxy.SendCommandAsync(request);
            return (result as Message.ListAppsResponse).Apps;
        }

        public async Task InstallAppAsync(Message.AppInstallInfo appInstallInfo)
        {
            var request = new Message.AppInstallRequest(appInstallInfo);
            var result = await this._systemConfiguratorProxy.SendCommandAsync(request);
            if (result.Status != Message.ResponseStatus.Success)
            {
                throw new Exception();
            }
        }

        public async Task UninstallAppAsync(Message.AppUninstallInfo appUninstallInfo)
        {
            var request = new Message.AppUninstallRequest(appUninstallInfo);
            var result = await this._systemConfiguratorProxy.SendCommandAsync(request);
            if (result.Status != Message.ResponseStatus.Success)
            {
                throw new Exception();
            }
        }

        public async Task<string> GetStartupForegroundAppAsync()
        {
            var request = new Message.GetStartupForegroundAppRequest();
            var result = await this._systemConfiguratorProxy.SendCommandAsync(request);
            return (result as Message.GetStartupForegroundAppResponse).StartupForegroundApp;
        }

        public async Task<IList<string>> ListStartupBackgroundAppsAsync()
        {
            var request = new Message.ListStartupBackgroundAppsRequest();
            var result = await this._systemConfiguratorProxy.SendCommandAsync(request);
            return (result as Message.ListStartupBackgroundAppsResponse).StartupBackgroundApps;
        }

        public async Task AddStartupAppAsync(Message.StartupAppInfo startupAppInfo)
        {
            var request = new Message.AddStartupAppRequest(startupAppInfo);
            var result = await this._systemConfiguratorProxy.SendCommandAsync(request);
            if (result.Status != Message.ResponseStatus.Success)
            {
                throw new Exception();
            }
        }

        public async Task RemoveStartupAppAsync(Message.StartupAppInfo startupAppInfo)
        {
            var request = new Message.RemoveStartupAppRequest(startupAppInfo);
            var result = await this._systemConfiguratorProxy.SendCommandAsync(request);
            if (result.Status != Message.ResponseStatus.Success)
            {
                throw new Exception();
            }
        }

        public async Task AppLifecycleAsync(Message.AppLifecycleInfo appInfo)
        {
            var request = new Message.AppLifecycleRequest(appInfo);
            var result = await this._systemConfiguratorProxy.SendCommandAsync(request);
            if (result.Status != Message.ResponseStatus.Success)
            {
                throw new Exception();
            }
        }

        public async Task TransferFileAsync(Message.AzureFileTransferInfo transferInfo)
        {
            //
            // C++ Azure Blob SDK not supported for ARM, so use Service to copy file to/from
            // App's LocalData and then use C# Azure Blob SDK to transfer
            //
            StorageFile appLocalDataFile = await ApplicationData.Current.TemporaryFolder.CreateFileAsync("tmp", CreationCollisionOption.GenerateUniqueName);
            transferInfo.AppLocalDataPath = appLocalDataFile.Path;

            if (!transferInfo.Upload)
            {
                // use Azure C# Storage SDK to download file into App LocalData
                
                // Retrieve storage account from connection string.
                CloudStorageAccount storageAccount = CloudStorageAccount.Parse(transferInfo.ConnectionString);

                // Create the blob client.
                CloudBlobClient blobClient = storageAccount.CreateCloudBlobClient();

                // Retrieve a reference to a container.
                CloudBlobContainer container = blobClient.GetContainerReference(transferInfo.ContainerName);

                // Retrieve reference to a named blob.
                CloudBlockBlob blockBlob = container.GetBlockBlobReference(transferInfo.BlobName);

                // Save blob contents to a file.
                await blockBlob.DownloadToFileAsync(appLocalDataFile);
            }

            // use C++ service to copy file to/from App LocalData
            var request = new Message.AzureFileTransferRequest(transferInfo);
            var result = await this._systemConfiguratorProxy.SendCommandAsync(request);
            if (result.Status != Message.ResponseStatus.Success)
            {
                throw new Exception();
            }

            if (transferInfo.Upload)
            {
                // use Azure C# Storage SDK to upload file from App LocalData

                // Retrieve storage account from connection string.
                CloudStorageAccount storageAccount = CloudStorageAccount.Parse(transferInfo.ConnectionString);

                // Create the blob client.
                CloudBlobClient blobClient = storageAccount.CreateCloudBlobClient();

                // Retrieve a reference to a container.
                CloudBlobContainer container = blobClient.GetContainerReference(transferInfo.ContainerName);

                // Create the container if it doesn't already exist.
                await container.CreateIfNotExistsAsync();

                // Retrieve reference to a blob named "photo1.jpg".
                CloudBlockBlob blockBlob = container.GetBlockBlobReference(transferInfo.BlobName);

                // Save blob contents to a file.
                await blockBlob.UploadFromFileAsync(appLocalDataFile);
            }

            await appLocalDataFile.DeleteAsync();

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

        private async Task AppStartSelfUpdate()
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


        private Task<string> AppStartSelfUpdateMethodHandlerAsync(string jsonParam)
        {
            Debug.WriteLine("AppStartSelfUpdateMethodHandlerAsync");

            AppStartSelfUpdate();

            return Task.FromResult(JsonConvert.SerializeObject(new { response = "succeeded" }));
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
                                case "timeInfo":
                                    if (managementProperty.Value.Type == JTokenType.Object)
                                    {
                                        Debug.WriteLine("timeInfo = " + managementProperty.Value.ToString());

                                        // Default JsonConvert Deserializing changes ISO8601 date fields to "mm/dd/yyyy hh:mm:ss".
                                        // We need to preserve the ISO8601 since that's the format SystemConfigurator understands.
                                        // Because of that, we are not using:
                                        // Message.SetTimeInfo requestInfo = JsonConvert.DeserializeObject<Message.SetTimeInfo>(fieldsJson);

                                        Message.SetTimeInfo setTimeInfo = new Message.SetTimeInfo();
                                        JObject subProperties = (JObject)managementProperty.Value;
                                        setTimeInfo.NtpServer = (string)subProperties.Property("NtpServer").Value;
                                        setTimeInfo.TimeZoneBias = (int)subProperties.Property("TimeZoneBias").Value;
                                        setTimeInfo.TimeZoneDaylightBias = (int)subProperties.Property("TimeZoneDaylightBias").Value;
                                        DateTime daylightDate = DateTime.Parse(subProperties.Property("TimeZoneDaylightDate").Value.ToString());
                                        setTimeInfo.TimeZoneDaylightDate = daylightDate.ToString("yyyy-MM-ddTHH:mm:ssZ");
                                        setTimeInfo.TimeZoneDaylightName = (string)subProperties.Property("TimeZoneDaylightName").Value;
                                        setTimeInfo.TimeZoneStandardBias = (int)subProperties.Property("TimeZoneStandardBias").Value;
                                        DateTime standardDate = DateTime.Parse(subProperties.Property("TimeZoneStandardDate").Value.ToString());
                                        setTimeInfo.TimeZoneStandardDate = standardDate.ToString("yyyy-MM-ddTHH:mm:ssZ");
                                        setTimeInfo.TimeZoneStandardName = (string)subProperties.Property("TimeZoneStandardName").Value;

                                        Message.SetTimeInfoRequest request = new Message.SetTimeInfoRequest(setTimeInfo);
                                        this._systemConfiguratorProxy.SendCommandAsync(request);
                                    }
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

        private async Task<string> ReportAllMethodHandler(string jsonParam)
        {
            Debug.WriteLine("ReportAllMethodHandler");

            Message.TimeInfoResponse timeInfoResponse = await GetTimeInfoAsync();


            Dictionary<string, object> collection = new Dictionary<string, object>();
            collection["microsoft"] = new
            {
                management = new
                {
                    timeInfo = timeInfoResponse
#if false // TODO
            collection["deviceStatus"] = await GetDeviceStatusAsync();
            collection["rebootInfo"] = await GetRebootInfoAsync();
#endif
                }
            };

            _deviceTwin.ReportProperties(collection);

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
    }

}
