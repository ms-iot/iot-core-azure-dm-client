using System;
using System.Collections.Generic;
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

namespace DMDashboard
{
    public partial class MainWindow : Window
    {
        const string iotHubConnectionString = "HostName=GMilekaStd.azure-devices.net;SharedAccessKeyName=iothubowner;SharedAccessKey=GVdk7+o+V6KiCSl3hMifN4r74Rd25h44GNWv3jVcF0o=";

        public MainWindow()
        {
            InitializeComponent();
            _registryManager = RegistryManager.CreateFromConnectionString(iotHubConnectionString);

            _dispatcherTimer = new System.Windows.Threading.DispatcherTimer();
            _dispatcherTimer.Tick += new EventHandler(OnReadReportedProperties);
            _dispatcherTimer.Interval = new TimeSpan(0, 0, 5);  // every 5 seconds
            // _dispatcherTimer.Start();

            PopulateDevices();
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
            if (path == "time.current")
            {
                DeviceCurrentTime.Text = value;
            }
            else if (path == "time.zoneStandardName")
            {
                ZoneStandardName.Text = value;
            }
            else if (path == "time.zoneDaylightName")
            {
                ZoneDaytimeName.Text = value;
            }
            else if (path == "Memory.TotalMemory")
            {
                TotalMemoryMB.Text = value;
            }
            else if (path == "Memory.AvailableMemory")
            {
                AvailableMemoryMB.Text = value;
            }
            else if (path == "Battery.Level")
            {
                BatteryLevel.Text = value;
            }
            else if (path == "Battery.Status")
            {
                BatteryStatus.Text = value;
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

        private void OnApplyDesired(object sender, RoutedEventArgs e)
        {
            JObject rebootProperties = new JObject();

            JValue singleRebootTimeValue = new JValue(DesiredSingleRebootTime.Text);
            JProperty singleRebootTimeProperty = new JProperty("singleReboot", singleRebootTimeValue);
            rebootProperties.Add(singleRebootTimeProperty);

            JValue dailyRebootTimeValue = new JValue(DesiredDailyRebootTime.Text);
            JProperty dailyRebootTimeProperty = new JProperty("dailyReboot", dailyRebootTimeValue);
            rebootProperties.Add(dailyRebootTimeProperty);

            JProperty rebootProperty = new JProperty("reboot", rebootProperties);

            JObject desiredProperties = new JObject();
            desiredProperties.Add(rebootProperty);

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

        private void OnRefreshReported(object sender, RoutedEventArgs e)
        {
            ReadReportedProperties();
        }

        // Data members
        private RegistryManager _registryManager;
        private DeviceTwinAndMethod _deviceTwin;
        private DispatcherTimer _dispatcherTimer;

    }
}
