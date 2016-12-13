using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading.Tasks;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using Microsoft.Devices.Management;
using Microsoft.Azure.Devices.Client;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;

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

        public MainPage()
        {
            this.InitializeComponent();
            this.buttonStart.IsEnabled = true;
            this.buttonStop.IsEnabled = false;
            this.imageHot.Visibility = Visibility.Collapsed;

            DeviceClient deviceClient = null; // TODO

            DMClient = DeviceManagementClient.Create(
                new AzureIoTHubDeviceTwinProxy(deviceClient), 
                new ToasterDeviceManagementRequestHandler(this));
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

        private async void ResetSystem()
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

        private void OnSystemReset(object sender, RoutedEventArgs e)
        {
            ResetSystem();
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

        private void OnApplyDesired(object sender, RoutedEventArgs e)
        {
            JObject desiredProperties = new JObject();
            desiredProperties.Add(GetDesiredReboot());

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

        #endregion

    }
}
