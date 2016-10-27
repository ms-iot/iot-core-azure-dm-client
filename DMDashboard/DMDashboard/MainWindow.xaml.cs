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

        private JProperty GetDesiredUpdates()
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
            JObject rebootProperties = new JObject();
            {
                JValue singleRebootTimeValue = new JValue(DesiredSingleRebootTime.Text);
                JProperty singleRebootTimeProperty = new JProperty("singleReboot", singleRebootTimeValue);
                rebootProperties.Add(singleRebootTimeProperty);

                JValue dailyRebootTimeValue = new JValue(DesiredDailyRebootTime.Text);
                JProperty dailyRebootTimeProperty = new JProperty("dailyReboot", dailyRebootTimeValue);
                rebootProperties.Add(dailyRebootTimeProperty);
            }

            JProperty rebootProperty = new JProperty("reboot", rebootProperties);

            JObject desiredProperties = new JObject();
            desiredProperties.Add(rebootProperty);
            desiredProperties.Add(GetDesiredUpdates());

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
            System.Threading.Thread.Sleep(5 * 1000);
            ReadReportedProperties();
        }

        private void OnDeepReadReported(object sender, RoutedEventArgs e)
        {
            SendDeepRead();
        }

        private void OnExpandSystemInfo(object sender, RoutedEventArgs e)
        {
            if (SystemInfoGrid.Visibility == Visibility.Collapsed)
            {
                SystemInfoGrid.Visibility = Visibility.Visible;
            }
            else
            {
                SystemInfoGrid.Visibility = Visibility.Collapsed;
            }
        }

        private void OnExpandTime(object sender, RoutedEventArgs e)
        {
            if (TimeGrid.Visibility == Visibility.Collapsed)
            {
                TimeGrid.Visibility = Visibility.Visible;
            }
            else
            {
                TimeGrid.Visibility = Visibility.Collapsed;
            }
        }

        private void OnExpandReboot(object sender, RoutedEventArgs e)
        {
            if (RebootGrid.Visibility == Visibility.Collapsed)
            {
                RebootGrid.Visibility = Visibility.Visible;
            }
            else
            {
                RebootGrid.Visibility = Visibility.Collapsed;
            }
        }

        private void OnExpandUpdate(object sender, RoutedEventArgs e)
        {
            if (UpdateGrid.Visibility == Visibility.Collapsed)
            {
                UpdateGrid.Visibility = Visibility.Visible;
            }
            else
            {
                UpdateGrid.Visibility = Visibility.Collapsed;
            }
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
            GetDesiredUpdates();
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

        private void OnTest(object sender, RoutedEventArgs e)
        {
            GetDesiredUpdates();
        }

        // Data members
        private RegistryManager _registryManager;
        private DeviceTwinAndMethod _deviceTwin;
        private DispatcherTimer _dispatcherTimer;
        private List<UpdateInfo> _updateList;

    }
}
