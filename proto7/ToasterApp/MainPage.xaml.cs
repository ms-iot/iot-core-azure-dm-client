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

        // This is the application-specific method handler
        private string OnMethodReceived(string methodName, string payload)
        {
            string result;
            if (DeviceManagementClient.TryHandleMethod(methodName, payload, out result))
            {
                // DM took care of this method, we're done
                return result;
            }
            // OK, we need to handle it here:
            // work-work-work
            // done
            return "";
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

        private void buttonStart_Click(object sender, RoutedEventArgs e)
        {
            this.buttonStart.IsEnabled = false;
            this.buttonStop.IsEnabled = true;
            this.slider.IsEnabled = false;
            this.textBlock.Text = string.Format("Toasting at {0}%", this.slider.Value);
            this.imageHot.Visibility = Visibility.Visible;
        }

        private void buttonStop_Click(object sender, RoutedEventArgs e)
        {
            this.buttonStart.IsEnabled = true;
            this.buttonStop.IsEnabled = false;
            this.slider.IsEnabled = true;
            this.textBlock.Text = "Ready";
            this.imageHot.Visibility = Visibility.Collapsed;
        }

        private void button_System_Reset(object sender, RoutedEventArgs e)
        {
            DMClient.StartFactoryReset();
        }

        private async void button_Check_for_Updates(object sender, RoutedEventArgs e)
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
    }
}
