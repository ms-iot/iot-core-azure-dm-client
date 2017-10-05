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

using Microsoft.Devices.Management;
using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using Windows.Foundation.Diagnostics;

using Mock.Microsoft.Azure.Devices.Client;

namespace Mock.App
{
    class AppDeviceManagementRequestHandler : IDeviceManagementRequestHandler
    {
        DeviceClientApp appHandler;

        public AppDeviceManagementRequestHandler(DeviceClientApp appHandler)
        {
            this.appHandler = appHandler;
        }

        // Answer the question "is it OK to reboot the device"
        async Task<bool> IDeviceManagementRequestHandler.IsSystemRebootAllowed()
        {
            return await Task.FromResult<bool>(true);
        }
    }

    class DeviceClientApp
    {
        private const string JsonVersion = "$version";

        public DeviceManagementClient DMClient
        {
            get
            {
                return _deviceManagementClient;
            }
        }

        private async Task<string> GetConnectionStringAsync()
        {
            var tpmDevice = new TpmDevice();

            string connectionString = "";

            do
            {
                try
                {
                    // connectionString = await tpmDevice.GetConnectionStringAsync();
                    connectionString = "HostName=remote-monitoring-pcs.azure-devices.net;DeviceId=gmileka06;SharedAccessKey=7iKTI2S9w0tY65kx8Ble7fk649guK6oCNZ27FpWD8co=";
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

        private async Task ResetConnectionAsync(DeviceClient existingConnection, Twin twin)
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
            _deviceClient = DeviceClient.CreateFromConnectionString(twin, deviceConnectionString, TransportType.Mqtt);

            // IDeviceTwin abstracts away communication with the back-end.
            // AzureIoTHubDeviceTwinProxy is an implementation of Azure IoT Hub
            IDeviceTwin deviceTwin = new Mock.Lib.AzureIoTHubDeviceTwinProxy(_deviceClient, twin, ResetConnectionAsync, Logger.Log);

            // IDeviceManagementRequestHandler handles device management-specific requests to the app,
            // such as whether it is OK to perform a reboot at any givem moment, according the app business logic
            // AppDeviceManagementRequestHandler is the Toaster app implementation of the interface
            IDeviceManagementRequestHandler appRequestHandler = new AppDeviceManagementRequestHandler(this);

            // Create the DeviceManagementClient, the main entry point into device management
            this._deviceManagementClient = await DeviceManagementClient.CreateAsync(deviceTwin, appRequestHandler);

            // Set the callback for desired properties update. The callback will be invoked
            // for all desired properties -- including those specific to device management
            await _deviceClient.SetDesiredPropertyUpdateCallbackAsync(OnDesiredPropertyUpdated, null);

            // Tell the _deviceManagementClient to sync the device with the current desired state.
            await this._deviceManagementClient.ApplyDesiredStateAsync();

            Logger.Log("ResetConnectionAsync end", LoggingLevel.Verbose);
        }

        public async Task InitializeDeviceClientAsync(Twin twin)
        {
            while (true)
            {
                try
                {
                    await ResetConnectionAsync(null, twin);
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

        private async Task OnDesiredPropertyUpdated(TwinCollection twinProperties, object userContext)
        {
            Dictionary<string, object> desiredProperties = new Dictionary<string, object>();
            foreach (KeyValuePair<string, object> p in twinProperties)
            {
                desiredProperties[p.Key] = p.Value;
            }
            desiredProperties[JsonVersion] = twinProperties.Version;

            // Let the device management client process properties specific to device management
            await this._deviceManagementClient.ApplyDesiredStateAsync(desiredProperties);

            // Application developer can process all the top-level nodes here
            // return Task.CompletedTask;
        }

        DeviceClient _deviceClient;
        DeviceManagementClient _deviceManagementClient;
    }
}