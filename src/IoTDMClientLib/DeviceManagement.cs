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

using Windows.Foundation.Diagnostics;
using System.Threading.Tasks;
using Microsoft.Devices.Management.DMDataContract;

namespace Microsoft.Devices.Management
{
    // This is the main entry point into DM without Azure connectivity
    public class DeviceManagement
    {
        public struct WindowsUpdateStatus
        {
            public string installed;
            public string approved;
            public string failed;
            public string installable;
            public string pendingReboot;
            public string lastScanTime;
            public bool deferUpgrade;
        }

        private DeviceManagement(ISystemConfiguratorProxy systemConfiguratorProxy)
        {
            Logger.Log("Entering DeviceManagementClient constructor without Azure connection.", LoggingLevel.Verbose);

            this._systemConfiguratorProxy = systemConfiguratorProxy;
        }
   
        public static DeviceManagement CreateWithoutAzure()
        {
            Logger.Log("Creating Device Management objects without Azure connection.", LoggingLevel.Verbose);

            var systemConfiguratorProxy = new SystemConfiguratorProxy();
            DeviceManagement DeviceManagement = new DeviceManagement(systemConfiguratorProxy); ;
            
            return DeviceManagement;
        }

        public async Task<WindowsUpdateStatus> GetWindowsUpdateStatusAsync()
        {
            var request = new Message.GetWindowsUpdatesRequest();
            Message.GetWindowsUpdatesResponse windowsUpdatesResponse = await _systemConfiguratorProxy.SendCommandAsync(request) as Message.GetWindowsUpdatesResponse;

            WindowsUpdateStatus status;
            status.approved = windowsUpdatesResponse.configuration.approved;
            status.installable = windowsUpdatesResponse.configuration.installable;
            status.installed = windowsUpdatesResponse.configuration.installed;
            status.lastScanTime = windowsUpdatesResponse.configuration.lastScanTime;
            status.pendingReboot = windowsUpdatesResponse.configuration.pendingReboot;
            status.failed = windowsUpdatesResponse.configuration.failed;
            status.deferUpgrade = windowsUpdatesResponse.configuration.deferUpgrade; 
            return status;
        }

        public async Task StartFactoryResetAsync(bool clearTPM, string recoveryPartitionGUID)
        {
            var request = new Message.FactoryResetRequest();
            request.clearTPM = clearTPM;
            request.recoveryPartitionGUID = recoveryPartitionGUID;

            await _systemConfiguratorProxy.SendCommandAsync(request);
        }

        public async Task StartRemoteWipeAsync(bool clearTPM)
        {
            var request = new Message.RemoteWipeRequest();
            request.clearTPM = clearTPM;

            await _systemConfiguratorProxy.SendCommandAsync(request);
        }

        public async Task UsoStartInteractiveScanAsync()
        {
            var request = new Message.UsoClientCmdRequest();
            request.cmd = UsoClientCmdDataContract.JsonStartInteractiveScan;

            await _systemConfiguratorProxy.SendCommandAsync(request);
        }

        public async Task UsoRestartDeviceAsync()
        {
            var request = new Message.UsoClientCmdRequest();
            request.cmd = UsoClientCmdDataContract.JsonRestartDevice;

            await _systemConfiguratorProxy.SendCommandAsync(request);
        }

        // Data members
        ISystemConfiguratorProxy _systemConfiguratorProxy;
    }
}
