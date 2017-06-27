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
using Microsoft.Devices.Management.DMDataContract;

namespace DMDashboard
{
    public partial class MainWindow : Window
    {
        enum AppLifeCycleAction
        {
            startApp,
            stopApp
        }

        class AppLifeCycleParameters
        {
            public string pkgFamilyName;
            public string action;
        }

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

        private void OnExpandWindowsUpdatePolicy(object sender, RoutedEventArgs e)
        {
            ToggleUIElementVisibility(WindowsUpdatePolicyGrid);
        }

        private void OnExpandWindowsUpdates(object sender, RoutedEventArgs e)
        {
            ToggleUIElementVisibility(WindowsUpdatesGrid);
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
            RegistryManager registryManager = RegistryManager.CreateFromConnectionString(connectionString);

            // Avoid duplicates in the list
            DeviceListBox.ItemsSource = null;

            // Populate devices.
            IEnumerable<Device> devices = await registryManager.GetDevicesAsync(100);
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

        private void OnDeviceSelected(object sender, SelectionChangedEventArgs e)
        {
            string deviceIdString = (string)DeviceListBox.SelectedItem;
            ConnectedProperties.IsEnabled = false;
            if (!String.IsNullOrEmpty(deviceIdString))
            {
                _deviceTwin = new DeviceTwinAndMethod(ConnectionStringBox.Text, deviceIdString);
                ConnectedProperties.IsEnabled = true;
            }
            SelectedDeviceName.Text = deviceIdString;
        }

        private async void OnManageAppLifeCycle(AppLifeCycleAction appLifeCycleAction, string packageFamilyName)
        {
            AppLifeCycleParameters parameters = new AppLifeCycleParameters();
            parameters.action = appLifeCycleAction == AppLifeCycleAction.startApp ? "start" : "stop";
            parameters.pkgFamilyName = packageFamilyName;
            string parametersString = JsonConvert.SerializeObject(parameters);
            CancellationToken cancellationToken = new CancellationToken();
            DeviceMethodReturnValue result = await _deviceTwin.CallDeviceMethod("microsoft.management.manageAppLifeCycle", parametersString, new TimeSpan(0, 0, 30), cancellationToken);
            MessageBox.Show("Reboot Command Result:\nStatus: " + result.Status + "\nReason: " + result.Payload);
        }

        private void OnStartApplication(object sender, RoutedEventArgs e)
        {
            OnManageAppLifeCycle(AppLifeCycleAction.startApp, LifeCyclePkgFamilyName.Text);
        }

        private void OnStopApplication(object sender, RoutedEventArgs e)
        {
            OnManageAppLifeCycle(AppLifeCycleAction.stopApp, LifeCyclePkgFamilyName.Text);
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

        private void RebootInfoModelToUI(Microsoft.Devices.Management.RebootInfo.GetResponse rebootInfo)
        {
            LastRebootCmdTime.Text = rebootInfo.lastRebootCmdTime.ToString();
            LastRebootCmdStatus.Text = rebootInfo.lastRebootCmdStatus?.ToString();
            LastBootTime.Text = rebootInfo.lastBootTime.ToString();
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
                if (jsonProp.Name == "timeInfo" && jsonProp.Value.Type == JTokenType.Object)
                {
                        TimeReportedState.FromJson((JObject)jsonProp.Value);
                }
                if (jsonProp.Name == "certificates")
                {
                    Microsoft.Devices.Management.Certificates certificatesInfo = JsonConvert.DeserializeObject<Microsoft.Devices.Management.Certificates>(jsonProp.Value.ToString());
                    CertificatesInfoToUI(certificatesInfo);
                }
                else if (jsonProp.Name == "deviceInfo")
                {
                    Microsoft.Devices.Management.DeviceInfo deviceInfo = JsonConvert.DeserializeObject<Microsoft.Devices.Management.DeviceInfo>(jsonProp.Value.ToString());
                    DeviceStatusModelToUI(deviceInfo);
                }
                else if (jsonProp.Name == "rebootInfo")
                {
                    Debug.WriteLine(jsonProp.Value.ToString());
                    var rebootInfo = JsonConvert.DeserializeObject<Microsoft.Devices.Management.RebootInfo.GetResponse>(jsonProp.Value.ToString());
                    RebootInfoModelToUI(rebootInfo);
                }
                else if (jsonProp.Name == "windowsUpdatePolicy")
                {
                    Debug.WriteLine(jsonProp.Value.ToString());
                    ReportedWindowsUpdatePolicy.FromJson(jsonProp.Value);
                }
                else if (jsonProp.Name == "windowsUpdates")
                {
                    Debug.WriteLine(jsonProp.Value.ToString());
                    var info = JsonConvert.DeserializeObject<Microsoft.Devices.Management.WindowsUpdates.GetResponse>(jsonProp.Value.ToString());
                    WindowsUpdatesConfigurationToUI(info);
                }
                else if (jsonProp.Name == "apps")
                {
                    Debug.WriteLine(jsonProp.Value.ToString());
                    TheAppsStatus.AppsStatusJsonToUI(jsonProp.Value);
                }
                else if (jsonProp.Name == "deviceHealthAttestation")
                {
                    Debug.WriteLine(jsonProp.Value.ToString());
                    var jobj = JObject.Parse(jsonProp.Value.ToString());
                    DeviceHealthAttestationReportedState.FromJson(jobj);
                }
                else if (jsonProp.Name == "wifi")
                {
                    Debug.WriteLine(jsonProp.Value.ToString());
                    this.WifiReportedState.FromJson(jsonProp.Value);
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

        private void OnExpandDeviceInfo(object sender, RoutedEventArgs e)
        {
            ToggleUIElementVisibility(DeviceInfoGrid);
        }

        private void OnExpandDeviceHealthAttestation(object sender, RoutedEventArgs e)
        {
            ToggleUIElementVisibility(DeviceHealthAttestationGrid);
        }

        private void DeviceStatusModelToUI(Microsoft.Devices.Management.DeviceInfo deviceInfo)
        {
            DevInfoId.Text = deviceInfo.id;
            DevInfoManufacturer.Text = deviceInfo.manufacturer;
            DevInfoModel.Text = deviceInfo.model;
            DevInfoDmVer.Text = deviceInfo.dmVer;
            DevInfoLang.Text = deviceInfo.lang;
            DevInfoType.Text = deviceInfo.type;
            DevInfoOEM.Text = deviceInfo.oem;
            DevInfoHwVer.Text = deviceInfo.hwVer;
            DevInfoFwVer.Text = deviceInfo.fwVer;
            DevInfoOSVer.Text = deviceInfo.osVer;
            DevInfoPlatform.Text = deviceInfo.platform;
            DevInfoProcessorType.Text = deviceInfo.processorType;
            DevInfoRadioSwVer.Text = deviceInfo.radioSwVer;
            DevInfoDisplayResolution.Text = deviceInfo.displayResolution;
            DevInfoCommercializationOperator.Text = deviceInfo.commercializationOperator;
            DevInfoProcessorArchitecture.Text = deviceInfo.processorArchitecture;
            DevInfoName.Text = deviceInfo.name;
            DevInfoTotalStorage.Text = deviceInfo.totalStorage;
            DevInfoTotalMemory.Text = deviceInfo.totalMemory;
            DevInfoSecureBootState.Text = deviceInfo.secureBootState;
            DevInfoOSEdition.Text = deviceInfo.osEdition;
            DevInfoBatteryStatus.Text = deviceInfo.batteryStatus;
            DevInfoBatteryRemaining.Text = deviceInfo.batteryRemaining;
            DevInfoBatteryRuntime.Text = deviceInfo.batteryRuntime;
        }

        private async void RebootSystemAsync()
        {
            CancellationToken cancellationToken = new CancellationToken();
            DeviceMethodReturnValue result = await _deviceTwin.CallDeviceMethod("microsoft.management.immediateReboot", "{}", new TimeSpan(0, 0, 30), cancellationToken);
            MessageBox.Show("Reboot Command Result:\nStatus: " + result.Status + "\nReason: " + result.Payload);
        }

        private void OnRebootSystem(object sender, RoutedEventArgs e)
        {
            RebootSystemAsync();
        }

        private async void FactoryResetAsync()
        {
            var resetParams = new FactorResetParams();
            resetParams.clearTPM = DesiredClearTPM.IsChecked == true;
            resetParams.recoveryPartitionGUID = DesiredRecoveryPartitionGUID.Text;
            string resetParamsString = JsonConvert.SerializeObject(resetParams);

            Debug.WriteLine("Reset params : " + resetParamsString);

            CancellationToken cancellationToken = new CancellationToken();
            DeviceMethodReturnValue result = await _deviceTwin.CallDeviceMethod("microsoft.management.factoryReset", resetParamsString, new TimeSpan(0, 0, 30), cancellationToken);
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

        private Microsoft.Devices.Management.RebootInfo.SetParams UIToRebootInfoModel()
        {
            var rebootInfo = new Microsoft.Devices.Management.RebootInfo.SetParams();
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
            SetDesired(TimeDesiredState.ToJson());
        }

        private ExternalStorage UIToExternalStorageModel()
        {
            ExternalStorage externalStorage = new ExternalStorage();
            externalStorage.connectionString = AzureStorageConnectionString.Text;
            return externalStorage;
        }

        private void OnSetExternalStorageInfo(object sender, RoutedEventArgs e)
        {
            SetDesired(UIToExternalStorageModel().ToJson());
        }

        private void OnSetWindowsUpdatePolicyInfo(object sender, RoutedEventArgs e)
        {
            SetDesired(DesiredWindowsUpdatePolicy.ToJson());
        }

        private Microsoft.Devices.Management.WindowsUpdates.SetParams UIToWindowsUpdatesConfiguration()
        {
            var configuration = new Microsoft.Devices.Management.WindowsUpdates.SetParams();

            configuration.approved = DesiredApproved.Text;
            
            return configuration;
        }

        private void WindowsUpdatesConfigurationToUI(Microsoft.Devices.Management.WindowsUpdates.GetResponse configuration)
        {
            ReportedInstalled.Text = configuration.installed;
            ReportedApproved.Text = configuration.approved;
            ReportedFailed.Text = configuration.failed;
            ReportedInstallable.Text = configuration.installable;
            ReportedPendingReboot.Text = configuration.pendingReboot;
            ReportedLastScanTime.Text = configuration.lastScanTime;
            ReportedDeferUpgrade.IsChecked = configuration.deferUpgrade;
        }

        private void OnSetWindowsUpdatesInfo(object sender, RoutedEventArgs e)
        {
            SetDesired(UIToWindowsUpdatesConfiguration().ToJson());
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

        private void OnSetAppsConfiguration(object sender, RoutedEventArgs e)
        {
            SetDesired(TheAppsConfigurator.GetJSon());
        }

        private void OnSetAllDesiredProperties(object sender, RoutedEventArgs e)
        {
            StringBuilder json = new StringBuilder();

            json.Append(UIToExternalStorageModel().ToJson());
            json.Append(",");
            json.Append(TimeDesiredState.ToJson());
            json.Append(",");
            json.Append(UIToCertificateConfiguration().ToJson());
            json.Append(",");
            json.Append(UIToRebootInfoModel().ToJson());
            json.Append(",");
            json.Append(DesiredWindowsUpdatePolicy.ToJson());
            json.Append(",");
            json.Append(UIToWindowsUpdatesConfiguration().ToJson());
            json.Append(",");
            json.Append(DeviceHealthAttestationDesiredState.ToJson());
            json.Append(",");
            json.Append(WifiDesiredState.ToJson());

            SetDesired(json.ToString());
        }

        private void OnExpandApps(object sender, RoutedEventArgs e)
        {
            ToggleUIElementVisibility(AppsGrid);
        }

        private async void UploadAppx(string connectionString, string container, string appxLocalPath, string dep0LocalPath, string dep1LocalPath, string certLocalPath)
        {
            // Retrieve storage account from connection string.
            var storageAccount = CloudStorageAccount.Parse(connectionString);

            // Create the blob client.
            var blobClient = storageAccount.CreateCloudBlobClient();

            // Retrieve a reference to a container.
            var containerRef = blobClient.GetContainerReference(container);

            // Create the container if it doesn't already exist.
            await containerRef.CreateIfNotExistsAsync();

            // Appx
            {
                var blob = containerRef.GetBlockBlobReference(new FileInfo(appxLocalPath).Name);
                await blob.UploadFromFileAsync(appxLocalPath);
            }

            // Dep1
            if (!string.IsNullOrEmpty(dep0LocalPath))
            {
                var blob = containerRef.GetBlockBlobReference(new FileInfo(dep0LocalPath).Name);
                await blob.UploadFromFileAsync(dep0LocalPath);
            }

            // Dep2
            if (!string.IsNullOrEmpty(dep1LocalPath))
            {
                var blob = containerRef.GetBlockBlobReference(new FileInfo(dep1LocalPath).Name);
                await blob.UploadFromFileAsync(dep1LocalPath);
            }

            // Certificate
            if (!string.IsNullOrEmpty(certLocalPath))
            {
                var blob = containerRef.GetBlockBlobReference(new FileInfo(certLocalPath).Name);
                await blob.UploadFromFileAsync(certLocalPath);
            }
        }

        private async void DeviceHealthAttestationReportButtonAsync(object sender, RoutedEventArgs e)
        {
            DeviceMethodReturnValue result = await _deviceTwin.CallDeviceMethod(DeviceHealthAttestationDataContract.ReportNowMethodName, "{}", new TimeSpan(0, 0, 30), new CancellationToken());
        }

        private void DeviceHealthAttestationSetInfoButtonAsync(object sender, RoutedEventArgs e)
        {
            SetDesired(DeviceHealthAttestationDesiredState.ToJson());
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

        private void OnExpandWifi(object sender, RoutedEventArgs e)
        {
            ToggleUIElementVisibility(WifiGrid);
        }

        private void OnSetWifiConfiguration(object sender, RoutedEventArgs e)
        {
            SetDesired(WifiDesiredState.ToJson());
        }

        public async void ExportWifiProfileDetails(string profileName, string storageConnectionString, string storageContainer, string blobName)
        {
            var details = new GetWifiProfileDetailsParams();
            {
                details.profileName = profileName;
                details.connectionString = storageConnectionString;
                details.containerName = storageContainer;
                details.blobName = blobName;
            }
            var parametersJson = JsonConvert.SerializeObject(details);
            Debug.WriteLine(parametersJson);

            var cancellationToken = new CancellationToken();
            DeviceMethodReturnValue result = await this._deviceTwin.CallDeviceMethod("microsoft.management.getWifiDetails", parametersJson, new TimeSpan(0, 0, 30), cancellationToken);
            MessageBox.Show("Get Wifi Profile Details Command Result:\nStatus: " + result.Status + "\nReason: " + result.Payload);
        }

        private void PopulateExternalStorageFromJson(JObject jRoot)
        {
            JToken jToken = jRoot.SelectToken("properties.desired.microsoft.management.externalStorage.connectionString");
            if (jToken != null && jToken is JValue)
            {
                JValue jConnectionString = (JValue)jToken;
                AzureStorageConnectionString.Text = (string)jConnectionString;
            }
        }

        private void OnLoadProfile(object sender, RoutedEventArgs e)
        {
            OpenFileDialog dlg = new OpenFileDialog();
            dlg.DefaultExt = ".json";
            dlg.Filter = "json files (*.json)|*.json|All files (*.*)|*.*";
            bool? result = dlg.ShowDialog();
            if (result != true)
            {
                return;
            }

            object rootObject = JsonConvert.DeserializeObject(File.ReadAllText(dlg.FileName));
            if (!(rootObject is JObject))
            {
                MessageBox.Show("Invalid json file content!");
            }

            JObject jRoot = (JObject)rootObject;
            PopulateExternalStorageFromJson(jRoot);
            TheAppsConfigurator.PopulateFromJson(jRoot);
        }

        private DeviceTwinAndMethod _deviceTwin;
    }
}
