using System;
using System.Threading.Tasks;

namespace Microsoft.Devices.Management
{
    // TODO: this type will come from the Azure IoT SDK
    public enum DeviceTwinUpdateState
    {
        Complete,
        Partial,
    }

    // This is the main entry point into DM
    public class DeviceManagementClient
    {
        IDeviceManagementRequestHandler requestHandler;
        IDeviceTwin deviceTwin;

        // Test code: ask about Reboot:
        private async Task test()
        {
            var response = await this.requestHandler.IsSystemRebootAllowed();
        }

        // Ultimately, DeviceManagementClient will take an abstraction over DeviceClient to allow it to 
        // send reported properties. It will never receive using it
        private DeviceManagementClient(IDeviceTwin deviceTwin, IDeviceManagementRequestHandler requestHandler)
        {
            this.requestHandler = requestHandler;
            this.deviceTwin = deviceTwin;

            //test();
        }

        public static DeviceManagementClient Create(IDeviceTwin deviceTwin, IDeviceManagementRequestHandler requestHandler)
        {
            return new DeviceManagementClient(deviceTwin, requestHandler);
        }

        //
        // Set up property and method filters
        //
        public static bool TryHandleMethod(string methodName, string payload, out string response)
        {
            // Is this a method that must be handled by the DM client?
            bool isDMMethod = false; // TODO: implement filter based on method name
            if (isDMMethod)
            {
                // Handle the method
                response = "all good";
                return true;
            }
            else
            {
                // Not ours -- the user must handle this method
                response = string.Empty;
                return false;
            }
        }

        public static bool TryHandleProperty(DeviceTwinUpdateState updateState, string payload)
        {
            // Is this a desired property that must be handled by the DM client?
            bool isDMProperty = false; // TODO: implement filter based on payload
            if (isDMProperty)
            {
                // Handle the property
                return true;
            }
            else
            {
                // Not ours -- the user must handle this property
                return false;
            }
        }

        //
        // Commands:
        //

        // This command initiates factory reset of the device
        public async Task StartFactoryReset()
        {
            var request = new DMRequest();
            request.command = DMCommand.FactoryReset;

            // Here we might want to set some reported properties:
            // ReportProperties("We're about to start factory reset... If you don't hear from me again, I'm dead");

            DMResponse result = await SystemConfiguratorProxy.SendCommandAsync(request);
            if (result.status != 0)
            {
                throw new Exception();
            }
        }

        public async Task StartSystemReboot()
        {
            var request = new DMRequest();
            request.command = DMCommand.SystemReboot;

            DMResponse result = await SystemConfiguratorProxy.SendCommandAsync(request);
            if (result.status != 0)
            {
                throw new Exception();
            }
        }

        // This command checks if updates are available. 
        // TODO: work out complete protocol (find updates, apply updates etc.)
        public async Task<bool> CheckForUpdatesAsync()
        {
            var request = new DMRequest();
            request.command = DMCommand.CheckUpdates;

            var response = await SystemConfiguratorProxy.SendCommandAsync(request);

            return response.status == 1;    // 1 means "updates available"
        }

        //
        // Private utilities
        //

        // Report property to DT
        private void ReportProperties(string allJson)
        {
            deviceTwin.ReportProperties(allJson);
        }
    }

}
