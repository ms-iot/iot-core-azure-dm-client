using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;


using Microsoft.Azure.Devices;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using Microsoft.Devices.Management;
using Microsoft.WindowsAzure.Storage;       // Namespace for CloudStorageAccount
using Microsoft.WindowsAzure.Storage.Blob;  // Namespace for Blob storage types
using System.Configuration;
using Microsoft.Win32;
using System.IO;

namespace DMDashboard
{
    public partial class MainWindow : Window
    {
        class BlobInfo
        {
            public string Name { get; set; }
            public string Type { get; set; }
            public string Uri { get; set; }

            public BlobInfo(string name, string type, string uri)
            {
                this.Name = name;
                this.Type = type;
                this.Uri = uri;
            }
        }

        static string IotHubConnectionString = "IotHubConnectionString";
        static string StorageConnectionString = "StorageConnectionString";

        Configuration config = ConfigurationManager.OpenExeConfiguration(ConfigurationUserLevel.None);

        public MainWindow()
        {
            InitializeComponent();

            var connectionString = this.config.AppSettings.Settings[IotHubConnectionString];
            if (connectionString != null && !string.IsNullOrEmpty(connectionString.Value)) {
                ConnectionStringBox.Text = connectionString.Value;
            }

            connectionString = this.config.AppSettings.Settings[StorageConnectionString];
            if (connectionString != null && !string.IsNullOrEmpty(connectionString.Value))
            {
                StorageConnectionStringBox.Text = connectionString.Value;
            }
        }

        private void ToggleUIElementVisibility(UIElement element)
        {
            if (element.Visibility == Visibility.Collapsed)
            {
                element.Visibility = Visibility.Visible;
            }
            else
            {
                element.Visibility = Visibility.Collapsed;
            }
        }

        private void OnExpandTimeInfo(object sender, RoutedEventArgs e)
        {
            ToggleUIElementVisibility(TimeInfoGrid);
        }

        private async void ListDevices(string connectionString)
        {
            _registryManager = RegistryManager.CreateFromConnectionString(connectionString);

            // Avoid duplicates in the list
            DeviceListBox.Items.Clear();

            // Populate devices.
            IEnumerable<Device> deviceIds = await this._registryManager.GetDevicesAsync(100);
            foreach (var deviceId in deviceIds)
            {
                Debug.WriteLine("->" + deviceId.Id);
                DeviceListBox.Items.Add(deviceId.Id);
            }

            this.config.AppSettings.Settings[IotHubConnectionString].Value = connectionString;
            this.config.Save(ConfigurationSaveMode.Modified);
        }

        private void OnListDevices(object sender, RoutedEventArgs e)
        {
            ListDevices(ConnectionStringBox.Text);
        }

        private void OnDeviceConnect(object sender, RoutedEventArgs e)
        {
            string deviceIdString = (string)DeviceListBox.SelectedItem;
            _deviceTwin = new DeviceTwinAndMethod(ConnectionStringBox.Text, deviceIdString);
            ConnectedProperties.IsEnabled = true;
        }

        private void OnDeviceSelected(object sender, SelectionChangedEventArgs e)
        {
            DeviceConnectButton.IsEnabled = true;
        }

        private void ListContainers(string connectionString)
        {
            CloudStorageAccount storageAccount = CloudStorageAccount.Parse(connectionString);
            CloudBlobClient blobClient = storageAccount.CreateCloudBlobClient();

            ContainersList.Items.Clear();
            foreach (var container in blobClient.ListContainers("", ContainerListingDetails.None, null, null))
            {
                ContainersList.Items.Add(container.Name);
            }

            this.config.AppSettings.Settings[StorageConnectionString].Value = connectionString;
            this.config.Save(ConfigurationSaveMode.Modified);
        }

        private void OnListContainers(object sender, RoutedEventArgs e)
        {
            ListContainers(StorageConnectionStringBox.Text);
        }

        private void OnListBlobs(object sender, RoutedEventArgs e)
        {
            if (ContainersList.SelectedIndex == -1)
            {
                return;
            }

            CloudStorageAccount storageAccount = CloudStorageAccount.Parse(StorageConnectionStringBox.Text);
            CloudBlobClient blobClient = storageAccount.CreateCloudBlobClient();
            CloudBlobContainer container = blobClient.GetContainerReference((string)ContainersList.SelectedItem);

            List<BlobInfo> blobInfoList = new List<BlobInfo>();
            foreach (IListBlobItem item in container.ListBlobs(null, false))
            {
                if (item.GetType() == typeof(CloudBlockBlob))
                {
                    CloudBlockBlob blob = (CloudBlockBlob)item;
                    blobInfoList.Add(new BlobInfo(blob.Name, "BlockBlob", blob.Uri.ToString()));
                }
                else if (item.GetType() == typeof(CloudPageBlob))
                {
                    CloudPageBlob pageBlob = (CloudPageBlob)item;
                    blobInfoList.Add(new BlobInfo(pageBlob.Name, "PageBlob", pageBlob.Uri.ToString()));

                }
                else if (item.GetType() == typeof(CloudBlobDirectory))
                {
                    CloudBlobDirectory directoryBlob = (CloudBlobDirectory)item;
                    blobInfoList.Add(new BlobInfo("<dir>", "BlobDirectory", directoryBlob.Uri.ToString()));
                }
            }
            BlobsList.ItemsSource = blobInfoList;
        }

        private void TimeInfoModelToUI(TimeInfo timeInfo)
        {
            LocalTime.Text = timeInfo.LocalTime.ToString();
            NtpServer.Text = timeInfo.NtpServer;
            ReportedTimeZoneBias.Text = timeInfo.TimeZoneBias.ToString();
            ReportedTimeZoneStandardName.Text = timeInfo.TimeZoneStandardName;
            ReportedTimeZoneStandardDate.Text = timeInfo.TimeZoneStandardDate.ToString();
            ReportedTimeZoneStandardBias.Text = timeInfo.TimeZoneStandardBias.ToString();
            ReportedTimeZoneDaylightName.Text = timeInfo.TimeZoneDaylightName;
            ReportedTimeZoneDaylightDate.Text = timeInfo.TimeZoneDaylightDate.ToString();
            ReportedTimeZoneDaylightBias.Text = timeInfo.TimeZoneDaylightBias.ToString();
        }

        private void RebootInfoModelToUI(RebootInfo rebootInfo)
        {
            LastRebootCmdTime.Text = rebootInfo.lastRebootCmdTime.ToString();
            LastRebootTime.Text = rebootInfo.lastRebootTime.ToString();
            ReportedSingleRebootTime.Text = rebootInfo.singleRebootTime.ToString();
            ReportedDailyRebootTime.Text = rebootInfo.dailyRebootTime.ToString();
        }

        private async void ReadDTReported()
        {
            DeviceTwinData deviceTwinData = await _deviceTwin.GetDeviceTwinData();
            Debug.WriteLine("json = " + deviceTwinData.reportedPropertiesJson);

            JObject jsonObject = (JObject)JsonConvert.DeserializeObject(deviceTwinData.reportedPropertiesJson);
            foreach (JProperty jsonProp in jsonObject.Children())
            {
                if (jsonProp.Name == "timeInfo")
                {
                    TimeInfo timeInfo = JsonConvert.DeserializeObject<TimeInfo>(jsonProp.Value.ToString());
                    TimeInfoModelToUI(timeInfo);
                }
                else if (jsonProp.Name == "deviceStatus")
                {
                    Microsoft.Devices.Management.DeviceStatus deviceStatus = JsonConvert.DeserializeObject<Microsoft.Devices.Management.DeviceStatus>(jsonProp.Value.ToString());
                    DeviceStatusModelToUI(deviceStatus);
                }
                else if (jsonProp.Name == "rebootInfo")
                {
                    RebootInfo rebootInfo = JsonConvert.DeserializeObject<RebootInfo>(jsonProp.Value.ToString());
                    RebootInfoModelToUI(rebootInfo);
                }
            }
        }

        private void OnReadDTReported(object sender, RoutedEventArgs e)
        {
            ReadDTReported();
        }

        private void OnExpandReboot(object sender, RoutedEventArgs e)
        {
            ToggleUIElementVisibility(RebootGrid);
        }

        private void OnExpandFactoryReset(object sender, RoutedEventArgs e)
        {
            ToggleUIElementVisibility(FactoryResetGrid);
        }

        private void OnExpandDeviceStatus(object sender, RoutedEventArgs e)
        {
            ToggleUIElementVisibility(DevicestatusGrid);
        }

        private void DeviceStatusModelToUI(Microsoft.Devices.Management.DeviceStatus deviceStatus)
        {
            DevStatusSecureBootState.Text = deviceStatus.secureBootState.ToString();
            DevStatusIPAddressV4.Text = deviceStatus.macAddressIpV4;
            DevStatusIPAddressV6.Text = deviceStatus.macAddressIpV6;
            DevStatusIsConnected.IsChecked = deviceStatus.macAddressIsConnected;
            DevStatusMacAddressType.Text = deviceStatus.macAddressType.ToString();
            DevStatusOSEdition.Text = deviceStatus.osType;
            DevStatusBatteryStatus.Text = deviceStatus.batteryStatus.ToString();
            DevStatusBatteryRemaining.Text = deviceStatus.batteryRemaining.ToString();
            DevStatusBatteryRuntime.Text = deviceStatus.batteryRuntime.ToString();
        }

        private async void RebootSystemAsync()
        {
            CancellationToken cancellationToken = new CancellationToken();
            DeviceMethodReturnValue result = await _deviceTwin.CallDeviceMethod("microsoft.management.immediateReboot", "{}", new TimeSpan(0, 0, 30), cancellationToken);
            // ToDo: it'd be nice to show the result in the UI.
        }

        private void OnRebootSystem(object sender, RoutedEventArgs e)
        {
            RebootSystemAsync();
        }

        private async void FactoryResetAsync()
        {
            CancellationToken cancellationToken = new CancellationToken();
            DeviceMethodReturnValue result = await _deviceTwin.CallDeviceMethod("DoFactoryResetAsync", "{}", new TimeSpan(0, 0, 30), cancellationToken);
            // ToDo: it'd be nice to show the result in the UI.
        }

        private void OnFactoryReset(object sender, RoutedEventArgs e)
        {
            FactoryResetAsync();
        }

        private async void UpdateDTReportedAsync()
        {
            CancellationToken cancellationToken = new CancellationToken();
            DeviceMethodReturnValue result = await _deviceTwin.CallDeviceMethod("ReportAllPropertiesAsync", "{}", new TimeSpan(0, 0, 30), cancellationToken);
            // ToDo: it'd be nice to show the result in the UI.
        }

        private void OnUpdateDTReported(object sender, RoutedEventArgs e)
        {
            UpdateDTReportedAsync();
        }

        private TimeInfo UIToTimeInfoModel()
        {
            TimeInfo timeInfo = new TimeInfo();

            ComboBoxItem ntpServerItem = (ComboBoxItem)DesiredNtpServer.SelectedItem;
            timeInfo.NtpServer = (string)ntpServerItem.Content;

            timeInfo.TimeZoneBias = Int32.Parse(DesiredTimeZoneBias.Text);
            timeInfo.TimeZoneStandardName = DesiredTimeZoneStandardName.Text;
            timeInfo.TimeZoneStandardDate = DateTime.Parse(DesiredTimeZoneStandardDate.Text);
            timeInfo.TimeZoneStandardBias = Int32.Parse(DesiredTimeZoneStandardBias.Text);
            timeInfo.TimeZoneDaylightName = DesiredTimeZoneDaylightName.Text;
            timeInfo.TimeZoneDaylightDate = DateTime.Parse(DesiredTimeZoneDaylightDate.Text);
            timeInfo.TimeZoneDaylightBias = Int32.Parse(DesiredTimeZoneDaylightBias.Text);

            return timeInfo;
        }

        private RebootInfo UIToRebootInfoModel()
        {
            RebootInfo rebootInfo = new RebootInfo();
            if (!String.IsNullOrEmpty(DesiredSingleRebootTime.Text))
            {
                rebootInfo.singleRebootTime = DateTime.Parse(DesiredSingleRebootTime.Text);
            }
            if (!String.IsNullOrEmpty(DesiredSingleRebootTime.Text))
            {
                rebootInfo.dailyRebootTime = DateTime.Parse(DesiredDailyRebootTime.Text);
            }
            return rebootInfo;
        }

        private void OnSetTimeInfo(object sender, RoutedEventArgs e)
        {
            DesiredProperties desiredProperties = new DesiredProperties();
            desiredProperties.timeInfo = UIToTimeInfoModel();
            SetDesired(desiredProperties);
        }

        private void OnSetRebootInfo(object sender, RoutedEventArgs e)
        {
            DesiredProperties desiredProperties = new DesiredProperties();
            desiredProperties.rebootInfo = UIToRebootInfoModel();
            SetDesired(desiredProperties);
        }

        private void SetDesired(DesiredProperties desiredProperties)
        {
            PropertiesRoot propertiesRoot = new PropertiesRoot();
            propertiesRoot.desired = desiredProperties;

            string jsonString = "{ \"properties\" : " + JsonConvert.SerializeObject(propertiesRoot) + "}";

            Debug.WriteLine("---- Desired Properties ----");
            Debug.WriteLine(jsonString);

            // Task t is to avoid the 'not awaited' warning.
            Task t = _deviceTwin.UpdateTwinData(jsonString);
        }

        private void OnSetAllDesiredProperties(object sender, RoutedEventArgs e)
        {
            DesiredProperties desiredProperties = new DesiredProperties();
            desiredProperties.timeInfo = UIToTimeInfoModel();
            desiredProperties.rebootInfo = UIToRebootInfoModel();
            SetDesired(desiredProperties);
        }

        private RegistryManager _registryManager;
        private DeviceTwinAndMethod _deviceTwin;


        private void OnExpandAppInstall(object sender, RoutedEventArgs e)
        {
            ToggleUIElementVisibility(AppInstallGrid);
        }

        private string Browse()
        {
            var fileDialog = new OpenFileDialog();
            fileDialog.Filter = "appx files (*.appx)|*.appx|appxbundle files (*.appxbundle)|*.appxbundle";
            fileDialog.RestoreDirectory = true;

            var result = fileDialog.ShowDialog();
            if (result != null && result.Value)
            {
                return fileDialog.FileName;
            }

            return null;
        }

        private void OnAppxBrowse(object sender, RoutedEventArgs e)
        {
            var appxPath = Browse();
            if (appxPath != null)
            {
                AppAppxPath.Text = appxPath;
            }
        }

        private void OnDep1AppxBrowse(object sender, RoutedEventArgs e)
        {
            var appxPath = Browse();
            if (appxPath != null)
            {
                AppDep1AppxPath.Text = appxPath;
            }
        }

        private void OnDep2AppxBrowse(object sender, RoutedEventArgs e)
        {
            var appxPath = Browse();
            if (appxPath != null)
            {
                AppDep2AppxPath.Text = appxPath;
            }
        }

        private void AppInstallButtonActivation(object sender, TextChangedEventArgs e)
        {
            bool isAppxPathProvided = (!string.IsNullOrEmpty(AppAppxPath.Text) && File.Exists(AppAppxPath.Text));
            bool isConnectionStringProvided = (!string.IsNullOrEmpty(AppConnectionString.Text));
            bool isContainerProvided = (!string.IsNullOrEmpty(AppContainerName.Text));
            AppInstallButton.IsEnabled = (isAppxPathProvided && isConnectionStringProvided && isContainerProvided);
        }

        private async void AppInstallAsync(object sender, RoutedEventArgs e)
        {
            var cxnstr = AppConnectionString.Text;
            var container = AppContainerName.Text;
            var pfn = AppPackageFamilyName.Text;
            var appx = AppAppxPath.Text;
            var dep1 = AppDep1AppxPath.Text;
            var dep2 = AppDep2AppxPath.Text;

            // copy local file to Azure
            {
                // Retrieve storage account from connection string.
                var storageAccount = CloudStorageAccount.Parse(cxnstr);

                // Create the blob client.
                var blobClient = storageAccount.CreateCloudBlobClient();

                // Retrieve a reference to a container.
                var containerRef = blobClient.GetContainerReference(container);

                // Create the container if it doesn't already exist.
                await containerRef.CreateIfNotExistsAsync();

                // Appx
                {
                    var blob = containerRef.GetBlockBlobReference(new FileInfo(appx).Name);
                    await blob.UploadFromFileAsync(appx);
                }

                // Dep1
                if (!string.IsNullOrEmpty(dep1))
                {
                    var blob = containerRef.GetBlockBlobReference(new FileInfo(dep1).Name);
                    await blob.UploadFromFileAsync(dep1);
                }

                // Dep2
                if (!string.IsNullOrEmpty(dep2))
                {
                    var blob = containerRef.GetBlockBlobReference(new FileInfo(dep2).Name);
                    await blob.UploadFromFileAsync(dep2);
                }
            }


            // Invoke DM App Install
            CancellationToken cancellationToken = new CancellationToken();

            var blobFormat = "{{\"ConnectionString\":\"{0}\",\"ContainerName\":\"{1}\",\"BlobName\":\"{2}\"}}";
            var appJson = string.Format(blobFormat, cxnstr, container, new FileInfo(appx).Name);
            var depsJson = "";
            if (!string.IsNullOrEmpty(dep1))
            {
                depsJson += string.Format(blobFormat, cxnstr, container, new FileInfo(dep1).Name);

                if (!string.IsNullOrEmpty(dep2))
                {
                    depsJson += ", ";
                    depsJson += string.Format(blobFormat, cxnstr, container, new FileInfo(dep2).Name);
                }
            }

            var jsonFormat = "{{\"PackageFamilyName\":\"{0}\",\"Appx\":{1},\"Dependencies\":[{2}]}}";
            var json = string.Format(jsonFormat, pfn, appJson, depsJson);
            var jo = JsonConvert.DeserializeObject(json);
            DeviceMethodReturnValue result = await _deviceTwin.CallDeviceMethod("microsoft.management.appInstall", json, new TimeSpan(0, 2, 00), cancellationToken);
        }
    }
}
