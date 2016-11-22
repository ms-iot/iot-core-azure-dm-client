using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Microsoft.Devices.Management
{
    public class DeviceManagementClient
    {
        IDeviceManagementRequestHandler rebootRequest;
        private DeviceManagementClient(IDeviceManagementRequestHandler rebootRequest)
        {
            this.rebootRequest = rebootRequest;
        }

        public static DeviceManagementClient Create(IDeviceManagementRequestHandler rebootRequest)
        {
            return new DeviceManagementClient(rebootRequest);
        }

        //
        // Commands:
        //

        // This command initiates factory reset of the device
        public async Task StartFactoryReset()
        {
            var request = new dm_request();
            request.command = 1;
            await DMCommunicator.SendCommandAsync(request);
        }

        // This command checks if updates are available. 
        // TODO: work out complete protocol (find updates, apply updates etc.)
        public Task CheckForUpdatesAsync()
        {
            // TODO
            return Task.CompletedTask;
        }
    }

}
