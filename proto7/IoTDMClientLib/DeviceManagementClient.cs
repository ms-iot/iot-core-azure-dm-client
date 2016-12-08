using System;
using System.Threading.Tasks;
using System.Collections.Generic;

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
        // Constants
        public const string RebootMethod = "Reboot";

        // Types
        public struct DMMethodResult
        {
            public uint returnCode;
            public string response;
        }

        // Data members
        IDeviceManagementRequestHandler requestHandler;
        IDeviceTwin deviceTwin;
        Dictionary<string, Func<string, Task<DMMethodResult>>> supportedMethods;

        // Ultimately, DeviceManagementClient will take an abstraction over DeviceClient to allow it to 
        // send reported properties. It will never receive using it
        private DeviceManagementClient(IDeviceTwin deviceTwin, IDeviceManagementRequestHandler requestHandler)
        {
            this.requestHandler = requestHandler;
            this.deviceTwin = deviceTwin;
            this.supportedMethods = new Dictionary<string, Func<string, Task<DMMethodResult>>>();
            this.supportedMethods.Add(RebootMethod, HandleRebootAsync);
        }

        public static DeviceManagementClient Create(IDeviceTwin deviceTwin, IDeviceManagementRequestHandler requestHandler)
        {
            return new DeviceManagementClient(deviceTwin, requestHandler);
        }

        public bool IsDMMethod(string methodName)
        {
            return supportedMethods.ContainsKey(methodName);
        }

        private async Task<DMMethodResult> HandleRebootAsync(string request)
        {
            DMMethodResult result = new DMMethodResult();

            try
            {
                await StartSystemReboot();
                result.returnCode = 1;  // success
            }
            catch (Exception)
            {
                // returnCode is already set to 0 to indicate failure.
            }
            return result;
        }

        public async Task<DMMethodResult> HandleMethodAsync(string methodName, string payload)
        {
            if (!IsDMMethod(methodName))
            {
                throw new ArgumentException("Unknown method name: " + methodName);
            }

            return await supportedMethods[methodName](payload);
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
            SystemRebootRequestResponse rebootAllowed = await requestHandler.IsSystemRebootAllowed();
            if (rebootAllowed != SystemRebootRequestResponse.StartNow)
            {
                // ToDo: What should happen if the the user blocks the restart?
                //       We need to have a policy on when to ask again.
                return;
            }

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
