using System.Collections.Generic;

namespace Microsoft.Devices.Management
{
    public class OnPremDeviceTwinProxy : IDeviceTwin
    {
        public OnPremDeviceTwinProxy(/* something */)
        {
        }

        void IDeviceTwin.SetManagementClient(DeviceManagementClient deviceManagementClient)
        {
            // Set a pointer to the DeviceManagementClient for callback or other scenario.
        }

        void IDeviceTwin.ReportProperties(Dictionary<string, object> collection)
        {
            // Somehow send the property to the DT
        }

        void IDeviceTwin.RefreshConnection()
        {
            // Reconnect if needed
        }
    }
}