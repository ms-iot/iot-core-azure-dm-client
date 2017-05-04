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
using System.Diagnostics;
using System.Globalization;
using System.Runtime.InteropServices;
using System.Threading.Tasks;
using Windows.Networking.Connectivity;
using Windows.UI.Core;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;

namespace IoTDMBuild2017
{
    internal static class NativeTimeMethods
    {
        [DllImport("api-ms-win-core-sysinfo-l1-2-1.dll")]
        internal static extern void GetLocalTime(out SYSTEMTIME lpLocalTime);
    }

    [StructLayout(LayoutKind.Sequential)]
    internal struct SYSTEMTIME
    {
        [MarshalAs(UnmanagedType.U2)]
        internal short Year;


        [MarshalAs(UnmanagedType.U2)]
        internal short Month;


        [MarshalAs(UnmanagedType.U2)]
        internal short DayOfWeek;


        [MarshalAs(UnmanagedType.U2)]
        internal short Day;


        [MarshalAs(UnmanagedType.U2)]
        internal short Hour;


        [MarshalAs(UnmanagedType.U2)]
        internal short Minute;


        [MarshalAs(UnmanagedType.U2)]
        internal short Second;


        [MarshalAs(UnmanagedType.U2)]
        internal short Milliseconds;

        internal DateTime ToDateTime()
        {
            return new DateTime(Year, Month, Day, Hour, Minute, Second);
        }
    }

    public sealed partial class MainPage : Page
    {
        private DispatcherTimer timer;
        private DeviceManagementClient deviceManagementClient;

        private readonly string DeviceConnectionString = ConnectionStringProvider.Value;

        private void EnableDeviceManagementUI(bool connected)
        {
            this.message.Text = connected ? "Connection established." : "Waiting for connection ... ";
            UpdateDateTime();
        }

        public MainPage()
        {
            this.InitializeComponent();

            timer = new DispatcherTimer();
            timer.Tick += timer_Tick;
            timer.Interval = TimeSpan.FromSeconds(10);
            timer.Start();

            EnableDeviceManagementUI(false);

#pragma warning disable 4014
            this.InitializeDeviceClientAsync();
#pragma warning restore 4014

        }

        private async Task<string> GetConnectionStringAsync()
        {
            var tpmDevice = new TpmDevice(0);

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
                Debug.WriteLine("Waiting...");
                await Task.Delay(1000);

            } while (true);

            return connectionString;
        }
        private async Task<bool> WaitForConnection()
        {
            do
            {
                ConnectionProfile connections = NetworkInformation.GetInternetConnectionProfile();
                bool internet = connections != null && connections.GetNetworkConnectivityLevel() == NetworkConnectivityLevel.InternetAccess;
                if (internet) break;

                Debug.WriteLine("Waiting...");
                await Task.Delay(1000);
            } while (true);

            return true;
        }
        private async Task InitializeDeviceClientAsync()
        {
            // Ensure network connection
            bool isConnected = await WaitForConnection();

            // Get connection string when av
            // string deviceConnectionString = await GetConnectionStringAsync();
            string deviceConnectionString = "HostName=remote-monitoring-pcs.azure-devices.net;DeviceId=gmileka08;SharedAccessKey=CS+WfPVysZjaSZcvF2FdDWzhZSiE8p4sqKBNfhdpI3Y=";

            // Create DeviceClient. Application uses DeviceClient for telemetry messages, device twin
            // as well as device management
            DeviceClient deviceClient = DeviceClient.CreateFromConnectionString(deviceConnectionString, TransportType.Mqtt);

            // IDeviceTwin abstracts away communication with the back-end.
            // AzureIoTHubDeviceTwinProxy is an implementation of Azure IoT Hub
            IDeviceTwin deviceTwinProxy = new AzureIoTHubDeviceTwinProxy(deviceClient);

            // IDeviceManagementRequestHandler handles device management-specific requests to the app,
            // such as whether it is OK to perform a reboot at any givem moment, according the app business logic
            // ToasterDeviceManagementRequestHandler is the Toaster app implementation of the interface
            IDeviceManagementRequestHandler appRequestHandler = new ToasterDeviceManagementRequestHandler(this);

            // Create the DeviceManagementClient, the main entry point into device management
            this.deviceManagementClient = await DeviceManagementClient.CreateAsync(deviceTwinProxy, appRequestHandler);

            EnableDeviceManagementUI(true);

            // Set the callback for desired properties update. The callback will be invoked
            // for all desired properties -- including those specific to device management
            await deviceClient.SetDesiredPropertyUpdateCallback(OnDesiredPropertyUpdate, null);

            await this.deviceManagementClient.ReportAllDeviceProperties();
        }

        public Task OnDesiredPropertyUpdate(TwinCollection desiredProperties, object userContext)
        {
            // Let the device management client process properties specific to device management
            this.deviceManagementClient.ApplyDesiredStateAsync(desiredProperties);

            // Application developer can process all the top-level nodes here
            return Task.CompletedTask;
        }

        // This method may get called on the DM callback thread - not on the UI thread.
        public async Task<bool> YesNo(string question)
        {
            return true;
        }
        private void timer_Tick(object sender, object e)
        {
            UpdateDateTime();
        }
        private void UpdateDateTime()
        {
            // Using DateTime.Now is simpler, but the time zone is cached. So, we use a native method insead.
            SYSTEMTIME localTime;
            NativeTimeMethods.GetLocalTime(out localTime);

            DateTime t = localTime.ToDateTime();
            var timeString = t.ToString("t", CultureInfo.CurrentCulture) + Environment.NewLine + t.ToString("d", CultureInfo.CurrentCulture);
            timer1.Text = timeString;
        }
    }
}
