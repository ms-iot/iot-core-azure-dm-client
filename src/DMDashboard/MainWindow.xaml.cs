using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;
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

            Desired_RootCATrustedCertificates_Root.ShowCertificateDetails += ShowCertificateDetails;
            Desired_RootCATrustedCertificates_CA.ShowCertificateDetails += ShowCertificateDetails;
            Desired_RootCATrustedCertificates_TrustedPublisher.ShowCertificateDetails += ShowCertificateDetails;
            Desired_RootCATrustedCertificates_TrustedPeople.ShowCertificateDetails += ShowCertificateDetails;
            Desired_CertificateStore_CA_System.ShowCertificateDetails += ShowCertificateDetails;
            Desired_CertificateStore_Root_System.ShowCertificateDetails += ShowCertificateDetails;
            Desired_CertificateStore_My_User.ShowCertificateDetails += ShowCertificateDetails;
            Desired_CertificateStore_My_System.ShowCertificateDetails += ShowCertificateDetails;

            Reported_RootCATrustedCertificates_Root.ShowCertificateDetails += ShowCertificateDetails;
            Reported_RootCATrustedCertificates_Root.ExportCertificateDetails += ExportCertificateDetails;
            Reported_RootCATrustedCertificates_CA.ShowCertificateDetails += ShowCertificateDetails;
            Reported_RootCATrustedCertificates_TrustedPublisher.ShowCertificateDetails += ShowCertificateDetails;
            Reported_RootCATrustedCertificates_TrustedPeople.ShowCertificateDetails += ShowCertificateDetails;
            Reported_CertificateStore_CA_System.ShowCertificateDetails += ShowCertificateDetails;
            Reported_CertificateStore_Root_System.ShowCertificateDetails += ShowCertificateDetails;
            Reported_CertificateStore_My_User.ShowCertificateDetails += ShowCertificateDetails;
            Reported_CertificateStore_My_System.ShowCertificateDetails += ShowCertificateDetails;
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

        private void OnExpandAzureStorage(object sender, RoutedEventArgs e)
        {
            ToggleUIElementVisibility(AzureStorageGrid);
        }

        private void OnExpandCertificates(object sender, RoutedEventArgs e)
        {
            ToggleUIElementVisibility(CertificateStackPanel);
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
            IEnumerable<Device> devices = await this._registryManager.GetDevicesAsync(100);
            List<string> deviceIds = new List<string>();
            foreach (var device in devices)
            {
                Debug.WriteLine("->" + device.Id);
                deviceIds.Add(device.Id);
            }

            deviceIds.Sort();
            DeviceListBox.ItemsSource = deviceIds;

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

        private void TimeInfoModelToUI(Microsoft.Devices.Management.TimeInfo.GetResponse timeInfo)
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

        private void CertificateInfoToUI(string hashesString, CertificateSelector certificateSelector)
        {
            if (String.IsNullOrEmpty(hashesString))
            {
                return;
            }
            string[] hashes = hashesString.Split('/');
            Array.Sort<string>(hashes);
            if (certificateSelector != null)
            {
                List<CertificateSelector.CertificateData> certificateList = new List<CertificateSelector.CertificateData>();
                foreach (string hash in hashes)
                {
                    CertificateSelector.CertificateData certificateData = new CertificateSelector.CertificateData();
                    certificateData.Hash = hash;
                    certificateData.FileName = "<unknown>";
                    certificateList.Add(certificateData);
                }
                certificateSelector.SetCertificateList(certificateList);
            }
        }

        private void CertificatesInfoToUI(Microsoft.Devices.Management.Certificates certificatesInfo)
        {
            CertificateInfoToUI(certificatesInfo.Configuration.rootCATrustedCertificates_CA, Reported_RootCATrustedCertificates_CA);
            CertificateInfoToUI(certificatesInfo.Configuration.rootCATrustedCertificates_Root, Reported_RootCATrustedCertificates_Root);
            CertificateInfoToUI(certificatesInfo.Configuration.rootCATrustedCertificates_TrustedPublisher, Reported_RootCATrustedCertificates_TrustedPublisher);
            CertificateInfoToUI(certificatesInfo.Configuration.rootCATrustedCertificates_TrustedPeople, Reported_RootCATrustedCertificates_TrustedPeople);

            CertificateInfoToUI(certificatesInfo.Configuration.certificateStore_CA_System, Reported_CertificateStore_CA_System);
            CertificateInfoToUI(certificatesInfo.Configuration.certificateStore_Root_System, Reported_CertificateStore_Root_System);
            CertificateInfoToUI(certificatesInfo.Configuration.certificateStore_My_User, Reported_CertificateStore_My_User);
            CertificateInfoToUI(certificatesInfo.Configuration.certificateStore_My_System, Reported_CertificateStore_My_System);
        }

        private async void ReadDTReported()
        {
            DeviceTwinData deviceTwinData = await _deviceTwin.GetDeviceTwinData();
            Debug.WriteLine("json = " + deviceTwinData.reportedPropertiesJson);

            JObject jsonObject = (JObject)JsonConvert.DeserializeObject(deviceTwinData.reportedPropertiesJson);

            JToken microsoftNode;
            if (!jsonObject.TryGetValue("microsoft", out microsoftNode) || microsoftNode.Type != JTokenType.Object)
            {
                return;
            }
            JObject microsoftObject = (JObject)microsoftNode;

            JToken managementNode;
            if (!microsoftObject.TryGetValue("management", out managementNode) || managementNode.Type != JTokenType.Object)
            {
                return;
            }
            JObject managementObject = (JObject)managementNode;

            foreach (JProperty jsonProp in managementObject.Children())
            {
                if (jsonProp.Name == "timeInfo")
                {
                    Microsoft.Devices.Management.TimeInfo.GetResponse timeInfo = JsonConvert.DeserializeObject<Microsoft.Devices.Management.TimeInfo.GetResponse>(jsonProp.Value.ToString());
                    TimeInfoModelToUI(timeInfo);
                }
                if (jsonProp.Name == "certificates")
                {
                    Microsoft.Devices.Management.Certificates certificatesInfo = JsonConvert.DeserializeObject<Microsoft.Devices.Management.Certificates>(jsonProp.Value.ToString());
                    CertificatesInfoToUI(certificatesInfo);
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

        /*
        private void OnExpandFactoryReset(object sender, RoutedEventArgs e)
        {
            ToggleUIElementVisibility(FactoryResetGrid);
        }
        */

        private void OnExpandApplication(object sender, RoutedEventArgs e)
        {
            ToggleUIElementVisibility(ApplicationGrid);
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

        private async void StartAppSelfUpdate()
        {
            CancellationToken cancellationToken = new CancellationToken();
            DeviceMethodReturnValue result = await _deviceTwin.CallDeviceMethod("microsoft.management.startAppSelfUpdate", "{}", new TimeSpan(0, 0, 30), cancellationToken);
            StartAppSelfUpdateResult.Text = result.Payload;
        }

        private void OnStartAppSelfUpdate(object sender, RoutedEventArgs e)
        {
            StartAppSelfUpdate();
        }

        private async void UpdateDTReportedAsync()
        {
            CancellationToken cancellationToken = new CancellationToken();
            DeviceMethodReturnValue result = await _deviceTwin.CallDeviceMethod("microsoft.management.reportAllDeviceProperties", "{}", new TimeSpan(0, 0, 30), cancellationToken);
            // ToDo: it'd be nice to show the result in the UI.
        }

        private void OnUpdateDTReported(object sender, RoutedEventArgs e)
        {
            UpdateDTReportedAsync();
        }

        private Microsoft.Devices.Management.TimeInfo.SetParams UIToTimeInfoModel()
        {
            Microsoft.Devices.Management.TimeInfo.SetParams timeInfo = new Microsoft.Devices.Management.TimeInfo.SetParams();

            ComboBoxItem ntpServerItem = (ComboBoxItem)DesiredNtpServer.SelectedItem;
            timeInfo.NtpServer = (string)ntpServerItem.Content;

            timeInfo.TimeZoneBias = Int32.Parse(DesiredTimeZoneBias.Text);
            timeInfo.TimeZoneStandardName = DesiredTimeZoneStandardName.Text;
            timeInfo.TimeZoneStandardDate = DesiredTimeZoneStandardDate.Text;
            timeInfo.TimeZoneStandardBias = Int32.Parse(DesiredTimeZoneStandardBias.Text);
            timeInfo.TimeZoneDaylightName = DesiredTimeZoneDaylightName.Text;
            timeInfo.TimeZoneDaylightDate = DesiredTimeZoneDaylightDate.Text;
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
            if (!String.IsNullOrEmpty(DesiredDailyRebootTime.Text))
            {
                rebootInfo.dailyRebootTime = DateTime.Parse(DesiredDailyRebootTime.Text);
            }
            return rebootInfo;
        }

        private void SetDesired(string sectionString)
        {
            string prefix = "{ \"properties\" : {\"desired\":{\"microsoft\":{\"management\":{";
            string suffix = "}}}}}";
            string jsonString = prefix + sectionString + suffix; // "{ \"properties\" : " + JsonConvert.SerializeObject(root) + "}";
            Debug.WriteLine("---- Desired Properties ----");
            Debug.WriteLine(jsonString);

            // Task t is to avoid the 'not awaited' warning.
            Task t = _deviceTwin.UpdateTwinData(jsonString);
        }

        private void OnSetTimeInfo(object sender, RoutedEventArgs e)
        {
            SetDesired(UIToTimeInfoModel().ToJson());
        }

        private ExternalStorage UIToExternalStorageModel()
        {
            ExternalStorage externalStorage = new ExternalStorage();
            externalStorage.connectionString = AzureStorageConnectionString.Text;
            externalStorage.container = AzureStorageContainerName.Text;
            return externalStorage;
        }

        private void OnSetExternalStorageInfo(object sender, RoutedEventArgs e)
        {
            SetDesired(UIToExternalStorageModel().ToJson());
        }

        private Certificates.CertificateConfiguration UIToCertificateConfiguration()
        {
            Certificates.CertificateConfiguration certificateConfiguration = new Certificates.CertificateConfiguration();
            certificateConfiguration.rootCATrustedCertificates_Root = Desired_RootCATrustedCertificates_Root.FileNamesString;
            certificateConfiguration.rootCATrustedCertificates_CA = Desired_RootCATrustedCertificates_CA.FileNamesString;
            certificateConfiguration.rootCATrustedCertificates_TrustedPublisher = Desired_RootCATrustedCertificates_TrustedPublisher.FileNamesString;
            certificateConfiguration.rootCATrustedCertificates_TrustedPeople = Desired_RootCATrustedCertificates_TrustedPeople.FileNamesString;
            certificateConfiguration.certificateStore_CA_System = Desired_CertificateStore_CA_System.FileNamesString;
            certificateConfiguration.certificateStore_Root_System = Desired_CertificateStore_Root_System.FileNamesString;
            certificateConfiguration.certificateStore_My_User = Desired_CertificateStore_My_User.FileNamesString;
            certificateConfiguration.certificateStore_My_System = Desired_CertificateStore_My_System.FileNamesString;
            return certificateConfiguration;
        }

        private void OnSetCertificateConfiguration(object sender, RoutedEventArgs e)
        {
            SetDesired(UIToCertificateConfiguration().ToJson());
        }

        private void OnSetRebootInfo(object sender, RoutedEventArgs e)
        {
            SetDesired(UIToRebootInfoModel().ToJson());
        }

        private void OnSetAllDesiredProperties(object sender, RoutedEventArgs e)
        {
            StringBuilder json = new StringBuilder();

            json.Append(UIToTimeInfoModel().ToJson());
            json.Append(",");
            json.Append(UIToExternalStorageModel().ToJson());
            json.Append(",");
            json.Append(UIToCertificateConfiguration().ToJson());
            json.Append(",");
            json.Append(UIToRebootInfoModel().ToJson());

            SetDesired(json.ToString());
        }

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
            DeviceMethodReturnValue result = await _deviceTwin.CallDeviceMethod("microsoft.management.appInstall", json, new TimeSpan(0, 0, 30), cancellationToken);
        }

        private void OnExpandAzureStorageExplorer(object sender, RoutedEventArgs e)
        {
            ToggleUIElementVisibility(AzureStorageExplorer);
        }

        private async Task<DeviceMethodReturnValue> RequestCertificateDetailsAsync(string connectionString, string containerName, string cspPath, string hash, string targetFileName)
        {
            GetCertificateDetailsParams getCertificateDetailsParams = new GetCertificateDetailsParams();
            getCertificateDetailsParams.path = cspPath;
            getCertificateDetailsParams.hash = hash;
            getCertificateDetailsParams.connectionString = connectionString;
            getCertificateDetailsParams.containerName = containerName;
            getCertificateDetailsParams.blobName = hash + ".json";
            string parametersJson = JsonConvert.SerializeObject(getCertificateDetailsParams);
            Debug.WriteLine(parametersJson);

            CancellationToken cancellationToken = new CancellationToken();
            return await _deviceTwin.CallDeviceMethod("microsoft.management.getCertificateDetails", parametersJson, new TimeSpan(0, 0, 30), cancellationToken);
        }

        private void ShowCertificateDetails(CertificateSelector sender, CertificateSelector.CertificateData certificateData)
        {
            CertificateDetails certificateDetails = new CertificateDetails();
            certificateDetails.Owner = this;
            certificateDetails.DataContext = certificateData;
            certificateDetails.ShowDialog();
        }

        private async void ExportCertificateDetailsAsync(CertificateSelector sender, CertificateSelector.CertificateData certificateData)
        {
            MessageBox.Show("Exporting certificate details from the device to Azure storage...");
            string targetFileName = certificateData.Hash + ".json";
            DeviceMethodReturnValue result = await RequestCertificateDetailsAsync(AzureStorageConnectionString.Text, AzureStorageContainerName.Text, sender.CertificatesPath, certificateData.Hash, targetFileName);
            GetCertificateDetailsResponse response = JsonConvert.DeserializeObject<GetCertificateDetailsResponse>(result.Payload);
            if (response == null || response.Status != 0)
            {
                MessageBox.Show("Error: could not schedule certificate export");
                return;
            }

            CertificateExportDetails.CertificateExportDetailsData certificateExportDetailsData = new CertificateExportDetails.CertificateExportDetailsData();
            certificateExportDetailsData.ConnectionString = AzureStorageConnectionString.Text;
            certificateExportDetailsData.ContainerName = AzureStorageContainerName.Text;
            certificateExportDetailsData.BlobName = targetFileName;

            CertificateExportDetails certificateExportDetails = new CertificateExportDetails();
            certificateExportDetails.Owner = this;
            certificateExportDetails.DataContext = certificateExportDetailsData;
            certificateExportDetails.Show();
        }

        private void ExportCertificateDetails(CertificateSelector sender, CertificateSelector.CertificateData certificateData)
        {
            ExportCertificateDetailsAsync(sender, certificateData);
        }

        private RegistryManager _registryManager;
        private DeviceTwinAndMethod _deviceTwin;
    }
}
