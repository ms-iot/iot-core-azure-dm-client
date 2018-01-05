using Microsoft.Azure.Devices;
using Microsoft.Devices.Management.DMDataContract;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using Windows.UI.Popups;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;

namespace DMController
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        const int NumberOfDevicesToPopulate = 50;
        const string DTRefreshing = "\"refreshing\"";
        const string DTRootNodeString = "{ \"properties\" : { \"desired\" : { \"" + DMJSonConstants.DTWindowsIoTNameSpace + "\" : ";
        const string DTRootNodeSuffixString = "}}}";

        const string DemoAppPackage = "DMClientOverview_jv0mnc6v5g12j"; //For demo purpose; Test uwp app (DMClientOverview) need to be installed on test device

        private DeviceTwinAndMethod _deviceTwin;
        private IEnumerable<Device> _deviceList;

        public MainPage()
        {
            this.InitializeComponent();

            //Set IoT Hub & Azure Storage connection string from container for app settings
            IoTHubConnTextBox.Text = App.IOTHUBCONNSTRING;
            StorageConnTextBox.Text = App.STORAGECONNSTRING;
            if (App.IOTHUBCONNSTRING.Length > 0)
            {
                ListDevices();
                SettingsGrid.Visibility = Visibility.Collapsed;
            }

            //Set root MainPage for all User Controls
            DeviceInfo.SetMainPage(this);
            Applications.SetMainPage(this);
            WindowsTelemetry.SetMainPage(this);
            WindowsUpdate.SetMainPage(this);
            Reboot.SetMainPage(this);
            TimeSettings.SetMainPage(this);
            Certificates.SetMainPage(this);
            DiagnosticLogs.SetMainPage(this);
            FactoryReset.SetMainPage(this);
            Wifi.SetMainPage(this);
            DHA.SetMainPage(this);
        }

        #region EventHandlers
        /// <summary>
        /// Change current selected Device.
        /// </summary>
        private void DeviceComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            string deviceIdString = (string)DeviceCombobox.SelectedItem;
            if (!String.IsNullOrEmpty(deviceIdString))
            {
                // Update _deviceTwin to selected device
                _deviceTwin = new DeviceTwinAndMethod(App.IOTHUBCONNSTRING, deviceIdString);
                if (_deviceTwin != null)
                {
                    // Update Device Info to selected device
                    DeviceInfo.SetCurrentDevice(deviceIdString);
                }

                Device device = _deviceList.FirstOrDefault(d => d.Id == deviceIdString);
                if (device != null)
                {
                    DeviceConnectionStatus.Text = device.ConnectionState.ToString();
                    DeleteDeviceButton.IsEnabled = true;
                }

            }
        }

        /// <summary>
        /// Delete current selected Device.
        /// </summary>
        private async void DeleteDeviceButton_ClickAsync(object sender, RoutedEventArgs e)
        {
            string deviceIdString = (string)DeviceCombobox.SelectedItem;
            if (!String.IsNullOrEmpty(deviceIdString))
            {
                ContentDialog deletePromptDialog = new ContentDialog
                {
                    Title = "Delete Device: " + deviceIdString,
                    Content = "Are you sure you want to delete device: " + deviceIdString + "?",
                    CloseButtonText = "Cancel",
                    PrimaryButtonText = "Delete"
                };
                deletePromptDialog.PrimaryButtonClick += async (s, args) =>
                {
                    ContentDialogButtonClickDeferral deferral = args.GetDeferral();
                    // Delete device
                    bool ifDeleted = await DeleteDeviceAsync(deviceIdString);
                    ListDevices();
                    deferral.Complete();
                };
                ContentDialogResult result = await deletePromptDialog.ShowAsync();
            }
        }

        /// <summary>
        /// Refresh device list with updated status.
        /// </summary>
        private void DeviceRefreshButton_Click(object sender, RoutedEventArgs e)
        {
            if (IoTHubConnTextBox.Text.Length == 0)
            {
                ShowDialogAsync("Settings Error", "IoT Hub connection string cannot be null");
                return;
            }
            ListDevices();
        }

        /// <summary>
        /// Show or hide Settings grid.
        /// </summary>
        private void SettingsButton_Click(object sender, RoutedEventArgs e)
        {
            if (SettingsGrid.Visibility == Windows.UI.Xaml.Visibility.Visible)
            {
                SettingsGrid.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
            }
            else
            {
                SettingsGrid.Visibility = Windows.UI.Xaml.Visibility.Visible;
            }
        }

        /// <summary>
        /// Save settings to container for app settings.
        /// </summary>
        private void SettingsSaveButton_Click(object sender, RoutedEventArgs e)
        {
            if (IoTHubConnTextBox.Text.Length == 0)
            {
                ShowDialogAsync("Settings Error", "IoT Hub connection string cannot be null");
                return;
            }
            App.IOTHUBCONNSTRING = IoTHubConnTextBox.Text;
            App.SetLocalSettings(App.AppSetting.IOTHUBCONNSTRING, App.IOTHUBCONNSTRING);
            App.STORAGECONNSTRING = StorageConnTextBox.Text;
            App.SetLocalSettings(App.AppSetting.STORAGECONNSTRING, App.STORAGECONNSTRING);

            SettingsGrid.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
            // Update device list from up
            ListDevices();
        }

        /// <summary>
        /// Test Function: Launch the demo UWP on all connected devices.
        /// </summary>
        private void TestStartAppButton_Click(object sender, RoutedEventArgs e)
        {
            CallDeviceMethodStartStopAll(DemoAppPackage, true); //start all DMOverview UWP 
        }

        /// <summary>
        /// Test Function: Stop the demo UWP on all connected devices.
        /// </summary>
        private void TestStopAppButton_Click(object sender, RoutedEventArgs e)
        {
            CallDeviceMethodStartStopAll(DemoAppPackage, false); //stop all DMOverview UWP 
        }
        #endregion EventHandlers

        #region PrivateFunctions
        /// <summary>
        /// Update the device list from IoT Hub.
        /// </summary>
        private async void ListDevices()
        {
            try
            {
                RegistryManager registryManager = RegistryManager.CreateFromConnectionString(App.IOTHUBCONNSTRING);

                // Avoid duplicates in the list
                DeviceCombobox.ItemsSource = null;

                // Populate devices.
                _deviceList = await registryManager.GetDevicesAsync(NumberOfDevicesToPopulate);

                List<string> deviceIds = new List<string>();
                foreach (var device in _deviceList)
                {
                    Debug.WriteLine("->" + device.Id);
                    deviceIds.Add(device.Id);
                }

                deviceIds.Sort();
                DeviceCombobox.ItemsSource = deviceIds;
                DeviceCombobox.SelectedIndex = 0;
            }
            catch (Exception e)
            {
                Debug.WriteLine("List Device Error: " + e.ToString());
                ShowDialogAsync("List Device Error", e.ToString());
            }
        }

        /// <summary>
        /// Delete a device from IoT Hub and update the device list.
        /// </summary>
        /// <param name="deviceIdString">The device ID to delete.</param>
        private async Task<bool> DeleteDeviceAsync(string deviceIdString)
        {
            try
            {
                RegistryManager registryManager = RegistryManager.CreateFromConnectionString(App.IOTHUBCONNSTRING);
                await registryManager.RemoveDeviceAsync(deviceIdString);
                return true;
            }
            catch (Exception ex)
            {
                Debug.WriteLine("Delete Device Error: " + ex.ToString());
                ShowDialogAsync("Delete Device Error", ex.ToString());
                return false;
            }
        }
        #endregion // PrivateFunctions

        #region PublicFunctions
        /// <summary>
        /// Retrieve Device Twin from given feature parameter. 
        /// </summary>
        /// <param name="windowsObj">The json section name.</param>
        /// <returns>Return the JToken object for given parameter.</returns>
        public async Task<JToken> GetTwinData(string windowsObj)
        {
            if (_deviceTwin == null)
            {
                ShowDialogAsync("Device Twin Error", "No device twin found. Please reconnect to IoT Hub.");
                return null;
            }

            JsonConvert.DefaultSettings = () => new JsonSerializerSettings
            {
                CheckAdditionalContent = false
            };
            DeviceTwinData deviceTwinData = await _deviceTwin.GetDeviceTwinData();

            if (deviceTwinData.reportedPropertiesJson == null)
            {
                ShowDialogAsync("Read Twin Error", windowsObj + ": Reported Twin in not valid");
                return null;
            }

            Debug.WriteLine("json = " + deviceTwinData.reportedPropertiesJson);
            JObject desiredObject = (JObject)JsonConvert.DeserializeObject(deviceTwinData.reportedPropertiesJson);

            JToken windowsToken;
            if (!desiredObject.TryGetValue(DMJSonConstants.DTWindowsIoTNameSpace, out windowsToken) || windowsToken.Type != JTokenType.Object)
            {
                return null;
            }
            JObject windowsObject = (JObject)windowsToken;
            var appsJson = windowsObject[windowsObj];
            if (appsJson != null)
            {
                return appsJson;
            }
            else
            {
                ShowDialogAsync("Read Twin Error", windowsObj + ": Reported Twin in not valid");
                return null;
            }
        }

        /// <summary>
        /// Update Device Twin
        /// </summary>
        /// <param name="refreshingValue">The json refresh string.</param>
        /// <param name="finalValue">The json final desired string.</param>
        public async Task UpdateTwinData(string refreshingValue, string finalValue)
        {
            if (_deviceTwin == null)
            {
                ShowDialogAsync("Device Twin Error", "No device twin found. Please reconnect to IoT Hub.");
                return;
            }

            JsonConvert.DefaultSettings = () => new JsonSerializerSettings
            {
                CheckAdditionalContent = false
            };

            Debug.WriteLine("---- Desired Properties ----");
            Debug.WriteLine(DTRootNodeString + refreshingValue + DTRootNodeSuffixString);
            await _deviceTwin.UpdateTwinData(DTRootNodeString + refreshingValue + DTRootNodeSuffixString);

            Debug.WriteLine("---- Desired Properties ----");
            Debug.WriteLine(DTRootNodeString + finalValue + DTRootNodeSuffixString);
            await _deviceTwin.UpdateTwinData(DTRootNodeString + finalValue + DTRootNodeSuffixString);

            ShowDialogAsync("Update Twin", "Desired state sent to Device Twin!");
        }

        /// <summary>
        /// Send direct methods to device.
        /// </summary>
        /// <param name="method">The direct method function name.</param>
        /// <param name="payload">The direct method payload.</param>
        public async Task<string> CallDeviceMethod(string method, string payload)
        {
            CancellationToken cancellationToken = new CancellationToken();
            DeviceMethodReturnValue result = await _deviceTwin.CallDeviceMethod(method, payload, new TimeSpan(0, 0, 30), cancellationToken);
            ShowDialogAsync("Call Device Method", "Request: " + method + "\n" + "Status: " + result.Status + "\nReason: " + result.Payload);
            return result.Payload;
        }

        /// <summary>
        /// Test Function: Send start / stop application direct method to all connected devices. 
        /// </summary>
        /// <param name="packageFamily">Package Family of the UWP to start / stop.</param>
        /// <param name="isStart">"true" to start, "false" to stop/> instance containing the event data.</param>
        private async void CallDeviceMethodStartStopAll(string packageFamily, bool isStart)
        {
            DeviceTwinAndMethod tempDeviceTwin;
            AppxLifeCycleDataContract.ManageAppLifeCycleParams parameters = new AppxLifeCycleDataContract.ManageAppLifeCycleParams();
            parameters.pkgFamilyName = packageFamily;
            if (isStart)
            {
                parameters.action = AppxLifeCycleDataContract.JsonStart;
            }
            else
            {
                parameters.action = AppxLifeCycleDataContract.JsonStop;
            }

            foreach (var device in _deviceList)
            {
                if (device.ConnectionState != DeviceConnectionState.Connected)
                {
                    continue; //skip devices that are not connected to IoT Hub
                }
                tempDeviceTwin = new DeviceTwinAndMethod(App.IOTHUBCONNSTRING, device.Id);
                if (tempDeviceTwin != null)
                {
                    CancellationToken cancellationToken = new CancellationToken();
                    DeviceMethodReturnValue result = await tempDeviceTwin.CallDeviceMethod(AppxLifeCycleDataContract.ManageAppLifeCycleAsync, parameters.ToJsonString(), new TimeSpan(0, 0, 30), cancellationToken);
                }
            }
        }

        /// <summary>
        /// Display a dialog box for alerts and errors.
        /// </summary>
        /// <param name="title">Title of dialog box.</param>
        /// <param name="content">Content of dialog box.</param>
        public async void ShowDialogAsync(string title, string content)
        {
            var dialog = new MessageDialog(content, title);
            dialog.Commands.Add(new UICommand("OK", delegate (IUICommand command)
            {
                // Do nothing
            }));
            await dialog.ShowAsync();
        }
        #endregion // PublicFunctions

    }
}
