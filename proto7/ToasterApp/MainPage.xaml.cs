using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Threading.Tasks;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;
using Windows.System;
using Windows.Storage.Streams;
using Windows.ApplicationModel;
using System.Text;
using Windows.ApplicationModel.Core;
using Windows.UI.Core;
using Microsoft.VisualStudio.Threading;
using Newtonsoft.Json;

using Microsoft.Devices.Management;
using System.Runtime.InteropServices;
using Microsoft.Azure.Devices.Client;

namespace Toaster
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        DeviceManagementClient DMClient;

        private async Task<DeviceManagementClient.DMMethodResult> HandleMethodCallAsync(string methodName, string payload)
        {
            DeviceManagementClient.DMMethodResult result = new DeviceManagementClient.DMMethodResult();
            if (DMClient.IsDMMethod(methodName))
            {
                result = await DMClient.HandleMethodAsync(methodName, payload);
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

<<<<<<< HEAD
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


        #endregion

        private async void OnListApps(object sender, RoutedEventArgs e)
        {
            bool success = true;
            try
            {
                await DMClient.StartListApps();
            }
            catch (Exception)
            {
                success = false;
            }

            StatusText.Text = success ? "Succeeded!" : "Failed!";
        }
    }
}
