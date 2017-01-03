using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading.Tasks;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using Microsoft.Azure.Devices.Client;
using Microsoft.Devices.Management;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using System.Text;

namespace Toaster
{
    public sealed partial class MainPage : Page
    {
        DeviceManagementClient DMClient;

        private async Task<DeviceManagementClient.DMMethodResult> HandleMethodCallAsync(string methodName, string payload)
        {
            DeviceManagementClient.DMMethodResult result = new DeviceManagementClient.DMMethodResult();
            if (DMClient.IsDMMethod(methodName))
            {
                result = await DMClient.InvokeMethodAsync(methodName, payload);
            }
            else
            {
                // Application may want to handle this method.

            }
            return result;
        }

        // This is the application-specific method handler
        private DeviceManagementClient.DMMethodResult OnMethodReceived(string methodName, string payload)
        {
            Task<DeviceManagementClient.DMMethodResult> result = HandleMethodCallAsync(methodName, payload);
            return result.Result;
        }

        private const string DeviceConnectionString = "<repalce>";

        public MainPage()
        {
            this.InitializeComponent();
            this.buttonStart.IsEnabled = true;
            this.buttonStop.IsEnabled = false;
            this.imageHot.Visibility = Visibility.Collapsed;

            DeviceClient deviceClient = DeviceClient.CreateFromConnectionString(DeviceConnectionString, TransportType.Mqtt);
            deviceClient.SetMethodHandler("CallMe", CallMe, null);

            DMClient = DeviceManagementClient.Create(
                new AzureIoTHubDeviceTwinProxy(deviceClient), 
                new ToasterDeviceManagementRequestHandler(this));
        }

        Task<MethodResponse> CallMe(MethodRequest methodRequest, object userContext)
        {
            // TODO: this will be hooked up with HandleMethodCallAsync etc. 
            // For now, just print out the JSON string and return a fake response
            // 'data="Toaster"'

            Debug.WriteLine(methodRequest.DataAsJson);

            var data = "Toaster";
            string result = "{\"data\":\"" + data + "\"}";
            var retValue = new MethodResponse(Encoding.UTF8.GetBytes(result), 200);

            return Task.FromResult(retValue);
        }

        public async Task<bool> YesNo(string question)
        {
            var dlg = new UserDialog(question);
            await dlg.ShowAsync();

            return dlg.Result;
        }

        private void OnStartToasting(object sender, RoutedEventArgs e)
        {
            this.buttonStart.IsEnabled = false;
            this.buttonStop.IsEnabled = true;
            this.slider.IsEnabled = false;
            this.textBlock.Text = string.Format("Toasting at {0}%", this.slider.Value);
            this.imageHot.Visibility = Visibility.Visible;
        }

        private void OnStopToasting(object sender, RoutedEventArgs e)
        {
            this.buttonStart.IsEnabled = true;
            this.buttonStop.IsEnabled = false;
            this.slider.IsEnabled = true;
            this.textBlock.Text = "Ready";
            this.imageHot.Visibility = Visibility.Collapsed;
        }

        private async void FactoryReset()
        {
            bool success = true;
            try
            {
                await DMClient.StartFactoryReset();
            }
            catch (Exception)
            {
                success = false;
            }

            StatusText.Text = success ? "Succeeded!" : "Failed!";
        }

        private void OnFactoryReset(object sender, RoutedEventArgs e)
        {
            FactoryReset();
        }

        private async void RestartSystem()
        {
            bool success = true;
            try
            {
                await DMClient.StartSystemReboot();
            }
            catch(Exception)
            {
                success = false;
            }

            StatusText.Text = success ?  "Succeeded!" : "Failed!";
        }

        private void OnSystemRestart(object sender, RoutedEventArgs e)
        {
            RestartSystem();
        }

        private async void OnCheckForUpdates(object sender, RoutedEventArgs e)
        {
            bool updatesAvailable = await DMClient.CheckForUpdatesAsync();
            if (updatesAvailable)
            {
                System.Diagnostics.Debug.WriteLine("updates available");
                var dlg = new UserDialog("Updates available. Install?");
                await dlg.ShowAsync();
                // Don't do anything yet
            }
        }

        // ----------------------------------------------------------------------------------------
        #region Device Twin callback simulation

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

        private void OnToggleDMGrid(object sender, RoutedEventArgs e)
        {
            if (DMGrid.Visibility == Visibility.Visible)
            {
                DMGrid.Visibility = Visibility.Collapsed;
            }
            else
            {
                DMGrid.Visibility = Visibility.Visible;
            }
        }

        private void OnExpandReboot(object sender, RoutedEventArgs e)
        {
            ToggleUIElementVisibility(RebootGrid);
        }

        private void OnExpandTimeInfo(object sender, RoutedEventArgs e)
        {
            ToggleUIElementVisibility(TimeInfoGrid);
        }

        // This is a mock method.
        // This will be replaced by a callback originating from the IoT Azure SDK
        // when it is ready.
        private void OnSystemRestartJson(object sender, RoutedEventArgs e)
        {
            OnMethodReceived(DeviceManagementClient.RebootMethod, "");
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

        private JProperty GetDesiredTimeInfo()
        {
            Microsoft.Devices.Management.JsonWriter jsonWriter = BuildTimeInfoJson();
            foreach (JProperty jsonProp in jsonWriter.Root.Children())
            {
                if (jsonProp.Name == "timeInfo")
                {
                    return jsonProp;
                }
            }
            return null;
        }

        private void OnApplyDesired(object sender, RoutedEventArgs e)
        {
            JObject desiredProperties = new JObject();
            desiredProperties.Add(GetDesiredReboot());
            desiredProperties.Add(GetDesiredTimeInfo());

            JProperty desiredProperty = new JProperty("desired", desiredProperties);

            JObject rootProperties = new JObject();
            rootProperties.Add(desiredProperty);

            JProperty rootProperty = new JProperty("properties", rootProperties);

            JObject rootObject = new JObject();
            rootObject.Add(rootProperty);

            string s = rootObject.ToString();

            Debug.WriteLine("---- Updating Reboot Desired Properties ----");
            Debug.WriteLine(s);

            DMClient.OnDesiredPropertiesChanged(DeviceTwinUpdateState.Complete, s);
        }

        private async void ReadRebootSingle()
        {
            ReportedSingleRebootTime.Text = await DMClient.GetPropertyAsync(DeviceManagementClient.ReportedRebootSingleProperty);
        }

        private void OnReadRebootSingle(object sender, RoutedEventArgs e)
        {
            ReadRebootSingle();
        }

        private async void ReadRebootDaily()
        {
            ReportedDailyRebootTime.Text = await DMClient.GetPropertyAsync(DeviceManagementClient.ReportedRebootDailyProperty);
        }

        private void OnReadRebootDaily(object sender, RoutedEventArgs e)
        {
            ReadRebootDaily();
        }

        private async void ReadLastRebootTime()
        {
            LastRebootTime.Text = await DMClient.GetPropertyAsync(DeviceManagementClient.ReportedLastRebootProperty);
        }

        private void OnReadLastRebootTime(object sender, RoutedEventArgs e)
        {
            ReadLastRebootTime();
        }

        private async void ReadLastRebootCmdTime()
        {
            LastRebootCmdTime.Text = await DMClient.GetPropertyAsync(DeviceManagementClient.ReportedLastRebootCmdProperty);
        }

        private void OnReadLastRebootCmdTime(object sender, RoutedEventArgs e)
        {
            ReadLastRebootCmdTime();
        }

        private async void GetTimeInfo()
        {
            DeviceManagementClient.TimeInfo timeInfo = await DMClient.GetTimeInfoAsync();
            LocalTime.Text = timeInfo.localTime.ToString();
            NtpServer.Text = timeInfo.ntpServer;
            ReportedTimeZoneBias.Text = timeInfo.timeZoneInformation.bias.ToString();
            ReportedTimeZoneStandardName.Text = timeInfo.timeZoneInformation.standardName;
            ReportedTimeZoneStandardDate.Text = timeInfo.timeZoneInformation.standardDate.ToString();
            ReportedTimeZoneStandardBias.Text = timeInfo.timeZoneInformation.standardBias.ToString();
            ReportedTimeZoneDaylightName.Text = timeInfo.timeZoneInformation.daylightName;
            ReportedTimeZoneDaylightDate.Text = timeInfo.timeZoneInformation.daylightDate.ToString();
            ReportedTimeZoneDaylightBias.Text = timeInfo.timeZoneInformation.daylightBias.ToString();
        }

        private void OnGetTimeInfo(object sender, RoutedEventArgs e)
        {
            GetTimeInfo();
        }

        private Microsoft.Devices.Management.JsonWriter BuildTimeInfoJson()
        {
            DeviceManagementClient.TimeInfo timeInfo = new DeviceManagementClient.TimeInfo();

            ComboBoxItem ntpServerItem = (ComboBoxItem)DesiredNtpServer.SelectedItem;
            timeInfo.ntpServer = (string)ntpServerItem.Content;

            timeInfo.timeZoneInformation.bias = Int32.Parse(DesiredTimeZoneBias.Text);
            timeInfo.timeZoneInformation.standardName = DesiredTimeZoneStandardName.Text;
            timeInfo.timeZoneInformation.standardDate = DateTime.Parse(DesiredTimeZoneStandardDate.Text);
            timeInfo.timeZoneInformation.standardBias = Int32.Parse(DesiredTimeZoneStandardBias.Text);
            timeInfo.timeZoneInformation.daylightName = DesiredTimeZoneDaylightName.Text;
            timeInfo.timeZoneInformation.daylightDate = DateTime.Parse(DesiredTimeZoneDaylightDate.Text);
            timeInfo.timeZoneInformation.daylightBias = Int32.Parse(DesiredTimeZoneDaylightBias.Text);

            /*
                {
                    "timeInfo":
                    {
                        "ntpServer": "pool.ntp.org",
                        "timeZone" :
                        {
                            "bias": 123,
                            "standardName": "(UTC-05:00) Eastern Time (US & Canada)",
                            "standardDate": "yyyy-mm-ddThh:mm:ss,day_of_week",
                            "standardBias": 33,
                            "daylightName": "(UTC-05:00) Eastern Time (US & Canada)",
                            "daylightDate": "yyyy-mm-ddThh:mm:ss,day_of_week",
                            "daylightBias": 33
                        }
                    }
                }
             */

            Microsoft.Devices.Management.JsonWriter jsonWriter = new Microsoft.Devices.Management.JsonWriter();
            jsonWriter.GetOrCreate("timeInfo.ntpServer").Value = timeInfo.ntpServer;
            jsonWriter.GetOrCreate("timeInfo.timeZone.bias").Value = timeInfo.timeZoneInformation.bias;
            jsonWriter.GetOrCreate("timeInfo.timeZone.standardName").Value = timeInfo.timeZoneInformation.standardName;
            jsonWriter.GetOrCreate("timeInfo.timeZone.standardDate").Value = timeInfo.timeZoneInformation.standardDate;
            jsonWriter.GetOrCreate("timeInfo.timeZone.standardBias").Value = timeInfo.timeZoneInformation.standardBias;
            jsonWriter.GetOrCreate("timeInfo.timeZone.daylightName").Value = timeInfo.timeZoneInformation.daylightName;
            jsonWriter.GetOrCreate("timeInfo.timeZone.daylightDate").Value = timeInfo.timeZoneInformation.daylightDate;
            jsonWriter.GetOrCreate("timeInfo.timeZone.daylightBias").Value = timeInfo.timeZoneInformation.daylightBias;

            return jsonWriter;
        }

        private async void SetTimeInfo()
        {
            Microsoft.Devices.Management.JsonWriter jsonWriter = BuildTimeInfoJson();
            DMClient.SetPropertyAsync(DeviceManagementClient.DesiredTimeInfoProperty, jsonWriter.JsonString);
        }

        private void OnSetTimeInfo(object sender, RoutedEventArgs e)
        {
            SetTimeInfo();
        }

        private void OnExpandDeviceStatus(object sender, RoutedEventArgs e)
        {
            ToggleUIElementVisibility(DevicestatusGrid);
        }

        private async void GetDeviceStatus()
        {
            DeviceManagementClient.DeviceStatus deviceStatus = await DMClient.GetDeviceStatusAsync();
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

        private void OnGetDeviceStatus(object sender, RoutedEventArgs e)
        {
            GetDeviceStatus();
        }

        private void OnExpandFactoryReset(object sender, RoutedEventArgs e)
        {
            ToggleUIElementVisibility(FactoryResetGrid);
        }

        #endregion

    }
}
