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
            DeviceManagement DeviceManagement = Create(systemConfiguratorProxy);
            
            return DeviceManagement;
        }

        internal static DeviceManagement Create(ISystemConfiguratorProxy systemConfiguratorProxy)
        {
            return new DeviceManagement(systemConfiguratorProxy);
        }

        public async Task<WindowsUpdateStatus> ReportWindowsUpdateStatus()
        {
            var request = new Message.GetWindowsUpdatesRequest();
            Message.GetWindowsUpdatesResponse windowsUpdatesResponse = await this._systemConfiguratorProxy.SendCommandAsync(request) as Message.GetWindowsUpdatesResponse;

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

        // Data members
        ISystemConfiguratorProxy _systemConfiguratorProxy;
    }
}
