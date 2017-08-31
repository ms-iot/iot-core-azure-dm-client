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
using Microsoft.Azure.Devices.Client;
using Microsoft.Azure.Devices.Shared;
using Microsoft.Devices.Management;
using System;
using System.Threading.Tasks;
using Windows.Foundation.Diagnostics;
using Windows.UI.Core;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;

namespace Toaster
{
    public sealed partial class MainPage : Page
    {
        DeviceManagementClient deviceManagementClient;

        private async Task EnableDeviceManagementUiAsync(bool enable)
        {
            await this.Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
            {
                this.buttonRestart.IsEnabled = enable;
                this.buttonReset.IsEnabled = enable;
            });
        }

        public MainPage()
        {
            this.InitializeComponent();
            this.buttonStart.IsEnabled = true;
            this.buttonStop.IsEnabled = false;

#pragma warning disable 4014
            // DM buttons will be enabled when we have created the DM client
            this.EnableDeviceManagementUiAsync(false);
            this.imageHot.Visibility = Visibility.Collapsed;

            this.InitializeDeviceClientAsync();
#pragma warning restore 4014

        }

        private async Task<string> GetConnectionStringAsync()
        {
            var tpmDevice = new TpmDevice();

            string connectionString = "";

            do
            {
                try
                {
                    connectionString = await tpmDevice.GetConnectionStringAsync();
                    break;
                }
                catch (Exception)
                {
                    // We'll just keep trying.
                }
                await Task.Delay(1000);

            } while (true);

            return connectionString;
        }

        private async Task ResetConnectionAsync(DeviceClient existingConnection)
        {
            Logger.Log("ResetConnectionAsync start", LoggingLevel.Verbose);
            // Attempt to close any existing connections before
            // creating a new one
            if (existingConnection != null)
            {
                await existingConnection.CloseAsync().ContinueWith((t) =>
                {
                    var e = t.Exception;
                    if (e != null)
                    {
                        var msg = "existingClient.CloseAsync exception: " + e.Message + "\n" + e.StackTrace;
                        System.Diagnostics.Debug.WriteLine(msg);
                        Logger.Log(msg, LoggingLevel.Verbose);
                    }
                });
            }

            // Get new SAS Token
            var deviceConnectionString = await GetConnectionStringAsync();

            // Create DeviceClient. Application uses DeviceClient for telemetry messages, device twin
            // as well as device management
            var newDeviceClient = DeviceClient.CreateFromConnectionString(deviceConnectionString, TransportType.Mqtt);

            // IDeviceTwin abstracts away communication with the back-end.
            // AzureIoTHubDeviceTwinProxy is an implementation of Azure IoT Hub
            IDeviceTwin deviceTwin = new AzureIoTHubDeviceTwinProxy(newDeviceClient, ResetConnectionAsync, Logger.Log);

            // IDeviceManagementRequestHandler handles device management-specific requests to the app,
            // such as whether it is OK to perform a reboot at any givem moment, according the app business logic
            // ToasterDeviceManagementRequestHandler is the Toaster app implementation of the interface
            IDeviceManagementRequestHandler appRequestHandler = new ToasterDeviceManagementRequestHandler(this);

            // Create the DeviceManagementClient, the main entry point into device management
            this.deviceManagementClient = await DeviceManagementClient.CreateAsync(deviceTwin, appRequestHandler);

            await EnableDeviceManagementUiAsync(true);

            // Set the callback for desired properties update. The callback will be invoked
            // for all desired properties -- including those specific to device management
            await newDeviceClient.SetDesiredPropertyUpdateCallbackAsync(OnDesiredPropertyUpdated, null);

            // Tell the deviceManagementClient to sync the device with the current desired state.
            await this.deviceManagementClient.ApplyDesiredStateAsync();

            Logger.Log("ResetConnectionAsync end", LoggingLevel.Verbose);
        }

        private async Task InitializeDeviceClientAsync()
        {
            while (true)
            {
                try
                {
                    await ResetConnectionAsync(null);
                    break;
                }
                catch (Exception e)
                {
                    var msg = "InitializeDeviceClientAsync exception: " + e.Message + "\n" + e.StackTrace;
                    System.Diagnostics.Debug.WriteLine(msg);
                    Logger.Log(msg, LoggingLevel.Error);
                }

                await Task.Delay(5 * 60 * 1000);
            }
        }

        public Task OnDesiredPropertyUpdated(TwinCollection desiredProperties, object userContext)
        {
            // Let the device management client process properties specific to device management
            this.deviceManagementClient.ApplyDesiredStateAsync(desiredProperties);

            // Application developer can process all the top-level nodes here
            return Task.CompletedTask;
        }

        // This method may get called on the DM callback thread - not on the UI thread.
        public async Task<bool> YesNo(string question)
        {
            var tcs = new TaskCompletionSource<bool>();

            await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, async () =>
            {
                UserDialog dlg = new UserDialog(question);
                ContentDialogResult dialogResult = await dlg.ShowAsync();
                tcs.SetResult(dlg.Result);
            });

            var result = await tcs.Task;
            return result;
        }

        private void OnStartToasting(object sender, RoutedEventArgs e)
        {
            this.buttonStart.IsEnabled = false;
            this.buttonStop.IsEnabled = true;
            this.slider.IsEnabled = false;
            this.textBlock.Text = string.Format("Toasting at {0}%", this.slider.Value);
            this.imageHot.Visibility = Visibility.Visible;

            if (deviceManagementClient != null)
            {
                deviceManagementClient.AllowReboots(false);
            }
        }

        private void OnStopToasting(object sender, RoutedEventArgs e)
        {
            if (deviceManagementClient != null)
            {
                deviceManagementClient.AllowReboots(true);
            }

            this.buttonStart.IsEnabled = true;
            this.buttonStop.IsEnabled = false;
            this.slider.IsEnabled = true;
            this.textBlock.Text = "Ready";
            this.imageHot.Visibility = Visibility.Collapsed;
        }
        /*  
        // ToDo: Not implemented in SystemConfigurator.
        private async void OnCheckForUpdates(object sender, RoutedEventArgs e)
        {
            bool updatesAvailable = await deviceManagementClient.CheckForUpdatesAsync();
            if (updatesAvailable)
            {
                System.Diagnostics.Debug.WriteLine("updates available");
                var dlg = new UserDialog("Updates available. Install?");
                await dlg.ShowAsync();
                // Don't do anything yet
            }
        }
        */
        private async void RestartSystem()
        {
            bool success = true;
            try
            {
                await deviceManagementClient.RebootAsync();
            }
            catch(Exception)
            {
                success = false;
            }

            StatusText.Text = success?  "Operation completed" : "Operation  failed";
        }

        private void OnSystemRestart(object sender, RoutedEventArgs e)
        {
            RestartSystem();
        }

        private async void FactoryReset()
        {
            bool success = true;
            try
            {
                // The recovery partition guid is typically picked from a pre-defined set of guids
                // by the builder of the image. For our testing purposes, we have been using the following
                // guid.
                string recoveryPartitionGUID = "a5935ff2-32ba-4617-bf36-5ac314b3f9bf";
                await deviceManagementClient.StartFactoryResetAsync(false /*don't clear TPM*/, recoveryPartitionGUID);
            }
            catch (Exception)
            {
                success = false;
            }

            StatusText.Text = success? "Succeeded!" : "Failed!";
         }

        private void OnFactoryReset(object sender, RoutedEventArgs e)
        {
            FactoryReset();
        }

        private async void SetWindowsTelemetryAsync()
        {
            try
            {
                await this.deviceManagementClient.SetWindowsTelemetryLevelAsync((WindowsTelemetryLevel)RequestedWindowsTelemetryLevel.SelectedIndex);
                StatusText.Text = "Set Windows Telemetry Level -> Success";
            }
            catch (Exception ex)
            {
                StatusText.Text = "Set Windows Telemetry Level -> Error: " + ex.HResult + " - " + ex.Message;
            }
        }

        private async void GetWindowsTelemetryAsync()
        {
            try
            {
                WindowsTelemetryLevel level = await this.deviceManagementClient.GetWindowsTelemetryLevelAsync();
                CurrentWindowsTelemetryLevel.Text = level.ToString();
                StatusText.Text = "Get Windows Telemetry Level -> Success";
            }
            catch (Exception ex)
            {
                StatusText.Text = "Get Windows Telemetry Level -> Error: " + ex.HResult + " - " + ex.Message;
            }
        }

        private void OnSetWindowsTelemetry(object sender, RoutedEventArgs e)
        {
            SetWindowsTelemetryAsync();
        }

        private void OnGetWindowsTelemetry(object sender, RoutedEventArgs e)
        {
            GetWindowsTelemetryAsync();
        }

        private async void SetTimeServiceStartedAsync()
        {
            try
            {
                TimeServiceState timeServiceState = new TimeServiceState();
                timeServiceState.enabled = true;
                timeServiceState.startup = ServiceStartup.Auto;
                timeServiceState.started = RequestedTimeServiceStartedState.SelectedIndex == 0;
                timeServiceState.settingsPriority = RequestedTimeServicePriorityState.SelectedIndex == 0 ? SettingsPriority.Local : SettingsPriority.Remote;

                await this.deviceManagementClient.SetTimeServiceAsync(timeServiceState);
                StatusText.Text = "Set Time Service Started -> Success";
            }
            catch (Exception ex)
            {
                StatusText.Text = "Set Time Service Started -> Error: " + ex.HResult + " - " + ex.Message;
            }
        }

        private async void GetTimeServiceStartedAsync()
        {
            try
            {
                TimeServiceState state = await this.deviceManagementClient.GetTimeServiceStateAsync();
                CurrentTimeServiceStartedState.Text = state.started ? "started" : "stopped";
                CurrentTimeServicePriorityState.Text = state.settingsPriority.ToString();
                StatusText.Text = "Get Time Service Started -> Success";
            }
            catch (Exception ex)
            {
                StatusText.Text = "Get Time Service Started -> Error: " + ex.HResult + " - " + ex.Message;
            }
        }

        private void OnSetTimeService(object sender, RoutedEventArgs e)
        {
            SetTimeServiceStartedAsync();
        }

        private void OnGetTimeServiceStarted(object sender, RoutedEventArgs e)
        {
            GetTimeServiceStartedAsync();
        }
    }
}
