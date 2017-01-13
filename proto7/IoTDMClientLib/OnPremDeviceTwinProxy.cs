// ToDo: need to update this layer to not take any dependencies on Microsoft.Azure.Devices.
using Microsoft.Azure.Devices.Client;
using Microsoft.Azure.Devices.Shared;


namespace Microsoft.Devices.Management
{
    public class OnPremDeviceTwinProxy : IDeviceTwin
    {
        public OnPremDeviceTwinProxy(/* something */)
        {
        }

        void IDeviceTwin.SetManagementClient(DeviceManagementClient deviceManagementClient)
        {
            // Somehow send the property to the DT
        }

        // ToDo: remove dependency on Azure type TwinCollection.
        void IDeviceTwin.ReportProperties(TwinCollection collection)
        {
            // Somehow send the property to the DT
        }

        // ToDo: remove dependency on Azure type MethodCallback.
        void IDeviceTwin.SetMethodHandler(string methodName, MethodCallback methodCallback, object userContext)
        {
            // Somehow send the property to the DT
        }

        void IDeviceTwin.RefreshConnection()
        {
            // Reconnect if needed
        }
    }
}