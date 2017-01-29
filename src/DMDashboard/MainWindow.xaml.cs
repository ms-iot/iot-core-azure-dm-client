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

namespace DMDashboard
{
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
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

            // Populate devices.
            IEnumerable<Device> deviceIds = await this._registryManager.GetDevicesAsync(100);
            foreach (var deviceId in deviceIds)
            {
                Debug.WriteLine("->" + deviceId.Id);
                DeviceListBox.Items.Add(deviceId.Id);
            }
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

        private void TimeInfoModelToUI(TimeInfo timeInfo)
        {
            LocalTime.Text = timeInfo.localTime.ToString();
            NtpServer.Text = timeInfo.ntpServer;
            ReportedTimeZoneBias.Text = timeInfo.timeZone.bias.ToString();
            ReportedTimeZoneStandardName.Text = timeInfo.timeZone.standardName;
            ReportedTimeZoneStandardDate.Text = timeInfo.timeZone.standardDate.ToString();
            ReportedTimeZoneStandardBias.Text = timeInfo.timeZone.standardBias.ToString();
            ReportedTimeZoneDaylightName.Text = timeInfo.timeZone.daylightName;
            ReportedTimeZoneDaylightDate.Text = timeInfo.timeZone.daylightDate.ToString();
            ReportedTimeZoneDaylightBias.Text = timeInfo.timeZone.daylightBias.ToString();
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
            timeInfo.ntpServer = (string)ntpServerItem.Content;

            timeInfo.timeZone.bias = Int32.Parse(DesiredTimeZoneBias.Text);
            timeInfo.timeZone.standardName = DesiredTimeZoneStandardName.Text;
            timeInfo.timeZone.standardDate = DateTime.Parse(DesiredTimeZoneStandardDate.Text);
            timeInfo.timeZone.standardBias = Int32.Parse(DesiredTimeZoneStandardBias.Text);
            timeInfo.timeZone.daylightName = DesiredTimeZoneDaylightName.Text;
            timeInfo.timeZone.daylightDate = DateTime.Parse(DesiredTimeZoneDaylightDate.Text);
            timeInfo.timeZone.daylightBias = Int32.Parse(DesiredTimeZoneDaylightBias.Text);

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
    }
}
