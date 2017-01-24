using Microsoft.Azure.Devices.Shared;
using Microsoft.WindowsAzure.Storage;
using Microsoft.WindowsAzure.Storage.Blob;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading.Tasks;
using Windows.Storage;

namespace Microsoft.Devices.Management
{
/*
    public class AzureFileTransfer
    {
        public string LocalPath { get; set; }
        public string AppLocalDataPath { get; set; }
        public string ConnectionString { get; set; }
        public string ContainerName { get; set; }
        public string BlobName { get; set; }
        public bool Upload { get; set; }
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

    public class AppLifecycleInfo
    {
        public string AppId { get; set; }
        public bool Start { get; set; }
    }

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

*/
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
        private DeviceManagementClient(IDeviceTwin deviceTwin, IDeviceManagementRequestHandler requestHandler, ISystemConfiguratorProxy systemConfiguratorProxy)
        {
            this._deviceTwin = deviceTwin;
            this._requestHandler = requestHandler;
            this._systemConfiguratorProxy = systemConfiguratorProxy;
        }

        public static DeviceManagementClient Create(IDeviceTwin deviceTwin, IDeviceManagementRequestHandler requestHandler)
        {
            DeviceManagementClient deviceManagementClient = Create(deviceTwin, requestHandler, new SystemConfiguratorProxy());
            deviceTwin.SetManagementClient(deviceManagementClient);
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
            var request = new Message.StartupAppRequest(startupAppInfo);
            var result = await this._systemConfiguratorProxy.SendCommandAsync(request);
            if (result.Status != Message.ResponseStatus.Success)
            {
                throw new Exception();
            }
        }

        public async Task RemoveStartupAppAsync(Message.StartupAppInfo startupAppInfo)
        {
            var request = new Message.StartupAppRequest(startupAppInfo);
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

        public async Task RebootSystemAsync()
        {
            if (await this._requestHandler.IsSystemRebootAllowed() == SystemRebootRequestResponse.StartNow)
            {
                var request = new Message.RebootRequest();
                await this._systemConfiguratorProxy.SendCommandAsync(request);
            }
        }

        public async Task<DMMethodResult> DoFactoryResetAsync()
        {
            throw new NotImplementedException();
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
                        SetPropertyAsync(Message.DMMessageKind.SetTimeInfo, valueString);
                    }
                }
                else if (dp.Key == "rebootInfo")
                {
                    if (!String.IsNullOrEmpty(valueString))
                    {
                        Debug.WriteLine(" rebootInfo json = ", valueString);
                        SetPropertyAsync(Message.DMMessageKind.SetRebootInfo, valueString);
                    }
                }
                else
                {
                    nonDMProperties[dp.Key] = dp.Value;
                }
            }

            return nonDMProperties;
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
