using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Windows.Threading;
using System.Diagnostics;
using Microsoft.Azure.Devices;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;

using Microsoft.Azure;                      // Namespace for CloudConfigurationManager
using Microsoft.WindowsAzure.Storage;       // Namespace for CloudStorageAccount
using Microsoft.WindowsAzure.Storage.Blob;  // Namespace for Blob storage types

namespace DMDashboard
{
    public partial class MainWindow : Window
    {
        const string iotHubConnectionString = "<connection string>";

        class UpdateInfo
        {
            public string ManifestFileName { get; set; }
            public bool IsUploaded { get; set; }
            public bool Download { get; set; }
            public bool Install { get; set; }
            public bool IsDownloaded { get; set; }
            public bool IsInstalled { get; set; }
        }

        public MainWindow()
        {
            InitializeComponent();

            _updateList = new List<UpdateInfo>();
            _registryManager = RegistryManager.CreateFromConnectionString(iotHubConnectionString);

            _dispatcherTimer = new System.Windows.Threading.DispatcherTimer();
            _dispatcherTimer.Tick += new EventHandler(OnReadReportedProperties);
            _dispatcherTimer.Interval = new TimeSpan(0, 0, 5);  // every 5 seconds
            // _dispatcherTimer.Start();

            PopulateDevices();
        }

        private void FillOutUpdateList(IList<string> blobNameList)
        {
            UpdateList.ItemsSource = null;

            _updateList.Clear();

            foreach (string blobName in blobNameList)
            {
                UpdateInfo data = new UpdateInfo();
                data.ManifestFileName = blobName;
                data.IsUploaded = true;
                data.Install = false;
                data.Download = false;
                data.IsInstalled = false;
                data.IsDownloaded = false;
                _updateList.Add(data);
            }

            UpdateList.ItemsSource = _updateList;
        }

        private void OnDeviceSelected(object sender, SelectionChangedEventArgs e)
        {
            ConnectButton.IsEnabled = true;
        }

        private void OnConnect(object sender, RoutedEventArgs e)
        {
            string deviceId = (string)DeviceListBox.SelectedItem;
            _deviceTwin = new DeviceTwinAndMethod(iotHubConnectionString, deviceId);
            ConnectedProperties.IsEnabled = true;
        }

        private void StringToListBox(string s, ListBox list)
        {
            string[] guids = s.Split('\\');
            list.Items.Clear();
            foreach (string guid in guids)
            {
                list.Items.Add(guid);
            }
        }

        private bool StringToBool(string s)
        {
            return s != "0";
        }

        private void UpdateUI(string path, string value)
        {
            if (path == "systemInfo.osVersion")
            {
                OSVersion.Text = value;
            }
            else if (path == "systemInfo.totalMemory")
            {
                TotalMemoryMB.Text = value;
            }
            else if (path == "systemInfo.availableMemory")
            {
                AvailableMemoryMB.Text = value;
            }
            else if (path == "systemInfo.totalStorage")
            {
                TotalStorageMB.Text = value;
            }
            else if (path == "systemInfo.availableStorage")
            {
                AvailableStorageMB.Text = value;
            }
            else if (path == "time.current")
            {
                DeviceCurrentTime.Text = value;
            }
            else if (path == "time.bias")
            {
                ZoneBias.Text = value;
            }
            else if (path == "time.zoneStandardName")
            {
                ZoneStandardName.Text = value;
            }
            else if (path == "time.zoneStandardDate")
            {
                ZoneStandardDate.Text = value;
            }
            else if (path == "time.zoneStandardBias")
            {
                ZoneStandardBias.Text = value;
            }
            else if (path == "time.zoneDaylightName")
            {
                ZoneDaytimeName.Text = value;
            }
            else if (path == "time.zoneDaylightDate")
            {
                ZoneDaytimeDate.Text = value;
            }
            else if (path == "time.zoneDaylightBias")
            {
                ZoneDaytimeBias.Text = value;
            }
            else if (path == "reboot.lastReboot")
            {
                LastRebootTime.Text = value;
            }
            else if (path == "reboot.lastRestart")
            {
                LastRestartTime.Text = value;
            }
            else if (path == "reboot.singleReboot")
            {
                ReportedSingleRebootTime.Text = value;
            }
            else if (path == "reboot.dailyReboot")
            {
                ReportedDailyRebootTime.Text = value;
            }
            else if (path == "remoteWipe.lastRemoteWipe")
            {
                LastRemoteWipeTime.Text = value;
            }
            else if (path == "windowsUpdate.allowAutoUpdate")
            {
                ReportedAllowAutoUpdate.IsChecked = StringToBool(value);
            }
            else if (path == "windowsUpdate.activeHours")
            {
                ReportedPolicyActiveHours.Text = value;
            }
            else if (path == "windowsUpdate.allowMUUpdates")
            {
                ReportedAllowMUUpdate.IsChecked = StringToBool(value);
            }
            else if (path == "windowsUpdate.allowNonMSsignedUpdates")
            {
                ReportedAllowNonMSSignedUpdate.IsChecked = StringToBool(value);
            }
            else if (path == "windowsUpdate.allowUpdateService")
            {
                ReportedAllowUpdateService.IsChecked = StringToBool(value);
            }
            else if (path == "windowsUpdate.branchReadinessLevel")
            {
                ReportedBranchReadinessLevel.Text = value;
            }
            else if (path == "windowsUpdate.deferFeatureUpdates")
            {
                ReportedDeferFeatureUpdates.Text = value;
            }
            else if (path == "windowsUpdate.deferQualityUpdates")
            {
                ReportedDeferQualityUpdates.Text = value;
            }
            else if (path == "windowsUpdate.excludeWUDriver")
            {
                ReportedExcludeWUDrivers.IsChecked = StringToBool(value);
            }
            else if (path == "windowsUpdate.pauseFeatureUpdates")
            {
                ReportedPauseFeatureUpdates.IsChecked = StringToBool(value);
            }
            else if (path == "windowsUpdate.pauseQualityUpdates")
            {
                ReportedPauseQualityUpdates.IsChecked = StringToBool(value);
            }
            else if (path == "windowsUpdate.requireUpdateApproval")
            {
                ReportedRequireUpdateApproval.IsChecked = StringToBool(value);
            }
            else if (path == "windowsUpdate.installDay")
            {
                ReportedScheduledInstallDay.Text = value;
            }
            else if (path == "windowsUpdate.installHour")
            {
                ReportedScheduledInstallHour.Text = value;
            }
            else if (path == "windowsUpdate.updateUrl")
            {
                ReportedUpdateURL.Text = value;
            }
            else if (path == "windowsUpdate.approvedUpdates")
            {
                StringToListBox(value, ReportedApprovedUpdates);
            }
            else if (path == "windowsUpdate.installedUpdates")
            {
                StringToListBox(value, ReportedInstalledUpdates);
            }
            else if (path == "windowsUpdate.failedUpdates")
            {
                StringToListBox(value, ReportedFailedUpdates);
            }
            else if (path == "windowsUpdate.installableUpdates")
            {
                StringToListBox(value, ReportedInstallableUpdates);
            }
            else if (path == "windowsUpdate.pendingRebootUpdates")
            {
                StringToListBox(value, ReportedPendingRebootUpdates);
            }
            else if (path == "windowsUpdate.lastSuccessfulScanTime")
            {
                ReportedLastScanTime.Text = value;
            }
            else if (path == "windowsUpdate.deferUpgrade")
            {
                ReportedDeferUpgrade.IsChecked = StringToBool(value);
            }
            else
            {
                string[] tokens = path.Split('.');
                // We are looking for "azureUpdates.<id>.state". So, 3 tokens exactly are expected.
                if (tokens.Length == 3)
                {
                    if (tokens[0] == "azureUpdates")
                    {
                        string id = tokens[1];
                        string fileName = "";
                        bool downloaded = false;
                        bool installed = false;

                        string[] valueTokens = value.Split(',');

                        // We are looking for "<filename>[,downloaded][,installed]
                        // Note that installed and downloaded can appear in any order.
                        uint valueIndex = 0;
                        foreach (string valueToken in valueTokens)
                        {
                            if (valueIndex == 0)
                            {
                                fileName = valueToken;
                            }
                            else
                            {
                                if (valueToken == "downloaded")
                                {
                                    downloaded = true;
                                }
                                else if (valueToken == "installed")
                                {
                                    installed = true;
                                }
                            }
                            ++valueIndex;
                        }
                        UpdateAzureUpdateEntry(id, fileName, downloaded, installed);
                    }
                }
            }
        }

        void UpdateAzureUpdateEntry(string id, string fileName, bool downloaded, bool installed)
        {
            UpdateList.ItemsSource = null;
            foreach (UpdateInfo updateInfo in _updateList)
            {
                if (updateInfo.ManifestFileName == fileName)
                {
                    updateInfo.IsInstalled = installed;
                    updateInfo.IsDownloaded = downloaded;
                    break;
                }
            }

            UpdateList.ItemsSource = _updateList;
        }

        private void ReadProperty(string indent, JProperty jsonProp)
        {
            indent += "    ";
            JTokenType type = jsonProp.Type;
            Debug.WriteLine(indent + jsonProp.Name + " = ");

            if (jsonProp.Value.Type == JTokenType.Object)
            {
                ReadObject(indent, (JObject)jsonProp.Value);
            }
            else
            {
                JValue theValue = (JValue)jsonProp.Value;
                Debug.WriteLine("Path = " + theValue.Path);
                switch (theValue.Type)
                {
                    case JTokenType.String:
                        {
                            string valueString = (string)theValue.Value;
                            UpdateUI(theValue.Path, valueString);
                            Debug.WriteLine(indent + valueString);
                        }
                        break;
                    case JTokenType.Date:
                        {
                            System.DateTime dateTime = (System.DateTime)theValue.Value;
                            UpdateUI(theValue.Path, dateTime.ToString());
                            Debug.WriteLine(indent + dateTime.ToString());
                        }
                        break;
                    case JTokenType.Integer:
                        {
                            long valueInt = (long)theValue.Value;
                            UpdateUI(theValue.Path, valueInt.ToString());
                            Debug.WriteLine(indent + valueInt);
                        }
                        break;
                }
            }
        }

        private void ReadObject(string indent, JObject jsonObj)
        {
            indent += "    ";
            foreach (JProperty child in jsonObj.Children())
            {
                ReadProperty(indent, child);
            }
        }

        private async void PopulateDevices()
        {
            IEnumerable<Device> deviceIds = await this._registryManager .GetDevicesAsync(100);
            foreach(var deviceId in deviceIds)
            {
                Debug.WriteLine("->" + deviceId.Id);
                DeviceListBox.Items.Add(deviceId.Id);
            }
        }

        private async void ReadReportedProperties()
        {
            DeviceTwinData dtd = await _deviceTwin.GetDeviceTwinData();
            string desiredProperties = dtd.desiredPropertiesJson;
            string reportedProperties = dtd.reportedPropertiesJson;

            JObject desiredObj = (JObject)JsonConvert.DeserializeObject(desiredProperties);
            JObject reportedObj = (JObject)JsonConvert.DeserializeObject(reportedProperties);

            ReadObject("", reportedObj);

            Debug.WriteLine("desired = " + desiredProperties);
            Debug.WriteLine("reported = " + reportedProperties);
        }

        private void OnReadReportedProperties(object sender, EventArgs e)
        {
            ReadReportedProperties();
        }

        string ToPropertyName(string s)
        {
            return s.Replace('.', '_').Replace('-', '_');
        }

        private JProperty GetDesiredReboot()
        {
            JObject rebootProperties = new JObject();
            {
                JValue singleRebootTimeValue = new JValue(DesiredSingleRebootTime.Text);
                JProperty singleRebootTimeProperty = new JProperty("singleReboot", singleRebootTimeValue);
                rebootProperties.Add(singleRebootTimeProperty);

                JValue dailyRebootTimeValue = new JValue(DesiredDailyRebootTime.Text);
                JProperty dailyRebootTimeProperty = new JProperty("dailyReboot", dailyRebootTimeValue);
                rebootProperties.Add(dailyRebootTimeProperty);
            }

            return new JProperty("reboot", rebootProperties);
        }


        private string GetListStrings(ListBox listBox)
        {
            string s = "";
            for (int i = 0; i < listBox.Items.Count; ++i)
            {
                string itemString = GetListBoxItemString(listBox.Items[i]);
                if (itemString == GuidPlaceHolder)
                {
                    continue;
                }

                if (s.Length != 0)
                {
                    s += @"\";
                }
                s += itemString;
            }
            return s;
        }

        private JProperty GetDesiredWindowsUpdates()
        {
            JObject properties = new JObject();
            {
                properties.Add(new JProperty("allowAutoUpdate", new JValue(DesiredAllowAutoUpdate.IsChecked == true ? 1 : 0)));
                properties.Add(new JProperty("activeHours", DesiredPolicyActiveHours.Text));
                properties.Add(new JProperty("allowMUUpdates", new JValue(DesiredAllowMUUpdate.IsChecked == true ? 1 : 0)));
                properties.Add(new JProperty("allowNonMSsignedUpdates", new JValue(DesiredAllowNonMSSignedUpdate.IsChecked == true ? 1 : 0)));
                properties.Add(new JProperty("allowUpdateService", new JValue(DesiredAllowUpdateService.IsChecked == true ? 1 : 0)));
                properties.Add(new JProperty("branchReadinessLevel", new JValue(Int32.Parse(DesiredBranchReadinessLevel.Text))));
                properties.Add(new JProperty("deferFeatureUpdates", new JValue(Int32.Parse(DesiredDeferFeatureUpdates.Text))));
                properties.Add(new JProperty("deferQualityUpdates", new JValue(Int32.Parse(DesiredDeferQualityUpdates.Text))));
                properties.Add(new JProperty("excludeWUDrivers", new JValue(DesiredExcludeWUDrivers.IsChecked == true ? 1 : 0)));
                properties.Add(new JProperty("pauseFeatureUpdates", new JValue(DesiredPauseFeatureUpdates.IsChecked == true ? 1 : 0)));
                properties.Add(new JProperty("pauseQualityUpdates", new JValue(DesiredPauseQualityUpdates.IsChecked == true ? 1 : 0)));
                properties.Add(new JProperty("requireUpdateApproval", new JValue(DesiredRequireUpdateApproval.IsChecked == true ? 1 : 0)));
                properties.Add(new JProperty("installDay", new JValue(Int32.Parse(DesiredScheduledInstallDay.Text))));
                properties.Add(new JProperty("installHour", new JValue(Int32.Parse(DesiredScheduledInstallHour.Text))));
                properties.Add(new JProperty("updateUrl", new JValue(DesiredUpdateURL.Text)));
                properties.Add(new JProperty("approvedUpdates", new JValue(GetListStrings(DesiredApprovedUpdates))));
            }

            return new JProperty("windowsUpdate", properties);
        }

        private JProperty GetDesiredAzureUpdates()
        {
            JObject updateProperties = new JObject();
            {
                JProperty connStrProperty = new JProperty("connStr", StorageConnectionString.Text);
                updateProperties.Add(connStrProperty);

                JProperty containerProperty = new JProperty("container", StorageContainer.Text);
                updateProperties.Add(containerProperty);

                JObject manifests = new JObject();

                foreach (UpdateInfo manifestData in _updateList)
                {
                    // Are there any operations desired?
                    if (manifestData.Download || manifestData.Install)
                    {
                        string parameters = manifestData.ManifestFileName;

                        if (manifestData.Download)
                        {
                            parameters += ",";
                            parameters += "download";
                        }

                        if (manifestData.Install)
                        {
                            parameters += ",";
                            parameters += "install";
                        }
                        // i.e. there's some action to do...
                        manifests.Add(new JProperty(ToPropertyName(manifestData.ManifestFileName), parameters));
                    }
                }
                updateProperties.Add(new JProperty("manifests", manifests));
            }

            JProperty updateProperty = new JProperty("azureUpdates", updateProperties);

            Debug.WriteLine("manifest info:\n" + updateProperty.ToString());

            return updateProperty;
        }

        private void OnApplyDesired(object sender, RoutedEventArgs e)
        {
            JObject desiredProperties = new JObject();
            desiredProperties.Add(GetDesiredReboot());
            desiredProperties.Add(GetDesiredWindowsUpdates());
            desiredProperties.Add(GetDesiredAzureUpdates());

            JProperty desiredProperty = new JProperty("desired", desiredProperties);

            JObject rootProperties = new JObject();
            rootProperties.Add(desiredProperty);

            JProperty rootProperty = new JProperty("properties", rootProperties);

            JObject rootObject = new JObject();
            rootObject.Add(rootProperty);

            string s = rootObject.ToString();

            Debug.WriteLine("---- Updating Reboot Desired Properties ----");
            Debug.WriteLine(s);

            // Task t is to avoid the 'not awaited' warning.
            Task t = _deviceTwin.UpdateTwinData(s);
        }

        private async void SendReboot()
        {
            CancellationToken cancellationToken = new CancellationToken();
            DeviceMethodReturnValue result = await _deviceTwin.CallDeviceMethod("Reboot", "{}", new TimeSpan(0, 0, 30), cancellationToken);
        }

        private void OnSendReboot(object sender, RoutedEventArgs e)
        {
            SendReboot();
        }

        private void OnReadReported(object sender, RoutedEventArgs e)
        {
            ReadReportedProperties();
        }

        private async void SendDeepRead()
        {
            CancellationToken cancellationToken = new CancellationToken();
            DeviceMethodReturnValue result = await _deviceTwin.CallDeviceMethod("Report", "{}", new TimeSpan(0, 0, 30), cancellationToken);
            await Task.Delay(5 * 1000);
            ReadReportedProperties();
        }

        private void OnDeepReadReported(object sender, RoutedEventArgs e)
        {
            SendDeepRead();
        }
        private async void SendRemoteWipe()
        {
            CancellationToken cancellationToken = new CancellationToken();
            DeviceMethodReturnValue result = await _deviceTwin.CallDeviceMethod("RemoteWipe", "{}", new TimeSpan(0, 0, 30), cancellationToken);
            await Task.Delay(5 * 1000);
            ReadReportedProperties();
        }

        private void OnSendRemoteWipe(object sender, RoutedEventArgs e)
        {
            SendRemoteWipe();
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

        private void OnExpandSystemInfo(object sender, RoutedEventArgs e)
        {
            ToggleUIElementVisibility(SystemInfoGrid);
        }

        private void OnExpandTime(object sender, RoutedEventArgs e)
        {
            ToggleUIElementVisibility(TimeGrid);
        }

        private void OnExpandReboot(object sender, RoutedEventArgs e)
        {
            ToggleUIElementVisibility(RebootGrid);
        }
        private void OnExpandRemoteWipe(object sender, RoutedEventArgs e)
        {
            ToggleUIElementVisibility(RemoteWipeGrid);
        }

        private void OnExpandWindowsUpdate(object sender, RoutedEventArgs e)
        {
            ToggleUIElementVisibility(WindowsUpdateGrid);
        }

        private void OnExpandAzureUpdate(object sender, RoutedEventArgs e)
        {
            ToggleUIElementVisibility(AzureUpdateGrid);
        }

        private List<string> GetBlobList(string connectionString, string containerName)
        {
            CloudStorageAccount storageAccount = CloudStorageAccount.Parse(connectionString);
            CloudBlobClient blobClient = storageAccount.CreateCloudBlobClient();
            CloudBlobContainer container = blobClient.GetContainerReference(containerName);

            List<string> blobNameList = new List<string>();
            foreach (IListBlobItem item in container.ListBlobs(null, false))
            {
                if (item.GetType() == typeof(CloudBlockBlob))
                {
                    CloudBlockBlob blob = (CloudBlockBlob)item;
                    Console.WriteLine("Block blob of length {0}: {1}", blob.Properties.Length, blob.Uri);
                    blobNameList.Add(blob.Name);

                }
                else if (item.GetType() == typeof(CloudPageBlob))
                {
                    CloudPageBlob pageBlob = (CloudPageBlob)item;
                    Console.WriteLine("Page blob of length {0}: {1}", pageBlob.Properties.Length, pageBlob.Uri);
                }
                else if (item.GetType() == typeof(CloudBlobDirectory))
                {
                    CloudBlobDirectory directory = (CloudBlobDirectory)item;
                    Console.WriteLine("Directory: {0}", directory.Uri);
                }
            }

            return blobNameList;
        }

        private void UpdateBlobList()
        {
            List<string> blobList = GetBlobList(StorageConnectionString.Text, StorageContainer.Text);
            FillOutUpdateList(blobList);
            GetDesiredAzureUpdates();
        }

        private void OnStorageConnect(object sender, RoutedEventArgs e)
        {
            UpdateBlobList();
        }

        /*
        void AzureStorageUpload(string connectionString, string containerName, string fullFileName)
        {
            CloudStorageAccount storageAccount = CloudStorageAccount.Parse(connectionString);
            CloudBlobClient blobClient = storageAccount.CreateCloudBlobClient();
            CloudBlobContainer container = blobClient.GetContainerReference(containerName);

            FileInfo fileInfo = new FileInfo(fullFileName);
            CloudBlockBlob blockBlob = container.GetBlockBlobReference(fileInfo.Name);
            using (var fileStream = System.IO.File.OpenRead(fullFileName))
            {
                blockBlob.UploadFromStream(fileStream);
            }
        }
        */

        private string GetListBoxItemString(object item)
        {
            string itemString = "";
            if (typeof(String) == item.GetType())
            {
                itemString = (string)item;
            }
            else
            {
                ListBoxItem listItem = (ListBoxItem)item;
                itemString = (string)listItem.Content;
            }
            return itemString;
        }

        private void OnAddApprovedUpdate(object sender, RoutedEventArgs e)
        {
            string firstItemString = GetListBoxItemString(DesiredApprovedUpdates.Items[0]);

            if (GuidPlaceHolder == firstItemString)
            {
                DesiredApprovedUpdates.Items.Clear();
            }
            DesiredApprovedUpdates.Items.Add(NewApprovedUpdateGuid.Text);
        }

        // Data members
        const string GuidPlaceHolder = "<none>";
        private RegistryManager _registryManager;
        private DeviceTwinAndMethod _deviceTwin;
        private DispatcherTimer _dispatcherTimer;
        private List<UpdateInfo> _updateList;
    }
}
