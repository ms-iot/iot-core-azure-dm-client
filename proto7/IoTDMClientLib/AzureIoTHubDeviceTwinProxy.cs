using Microsoft.Azure.Devices.Client;
using Microsoft.Azure.Devices.Shared;
using System.Text;
using System.Threading.Tasks;

namespace Microsoft.Devices.Management
{
    // This IDeviceTwin represents the actual Azure IoT Device Twin
    public class AzureIoTHubDeviceTwinProxy : IDeviceTwin
    {
        DeviceClient deviceClient;
        DeviceManagementClient deviceManagementClient;

        public AzureIoTHubDeviceTwinProxy(DeviceClient deviceClient)
        {
            this.deviceClient = deviceClient;

            this.deviceClient.SetMethodHandler("ReportAllPropertiesAsync", ReportAllPropertiesAsync, null);
            this.deviceClient.SetMethodHandler("DoFactoryResetAsync", DoFactoryResetAsync, null);
            this.deviceClient.SetMethodHandler("RebootSystemAsync", RebootSystemAsync, null);
        }

        public void SetManagementClient(DeviceManagementClient deviceManagementClient)
        {
            this.deviceManagementClient = deviceManagementClient;
        }

        void IDeviceTwin.ReportProperties(TwinCollection collection)
        {
            this.deviceClient.UpdateReportedPropertiesAsync(collection);
        }

        void IDeviceTwin.SetMethodHandler(string methodName, MethodCallback methodCallback, object userContext)
        {
            this.deviceClient.SetMethodHandler(methodName, methodCallback, userContext);
        }

        void IDeviceTwin.RefreshConnection()
        {
            // Recreate deviceClient using a new SAS token
        }

        Task<MethodResponse> ReportAllPropertiesAsync(MethodRequest methodRequest, object userContext)
        {
            if (deviceManagementClient == null)
            {
                throw new System.Exception("ManagementClient is not set.");
            }

            // We won't wait...
            deviceManagementClient.ReportAllPropertiesAsync();

            var retValue = new MethodResponse(Encoding.UTF8.GetBytes(""), 200 /*dm success code*/);
            return Task.FromResult(retValue);
        }

        Task<MethodResponse> DoFactoryResetAsync(MethodRequest methodRequest, object userContext)
        {
            if (deviceManagementClient == null)
            {
                throw new System.Exception("ManagementClient is not set.");
            }

            // We won't wait...
            deviceManagementClient.DoFactoryResetAsync();

            var retValue = new MethodResponse(Encoding.UTF8.GetBytes(""), 200 /*dm success code*/);
            return Task.FromResult(retValue);
        }

        Task<MethodResponse> RebootSystemAsync(MethodRequest methodRequest, object userContext)
        {
            if (deviceManagementClient == null)
            {
                throw new System.Exception("ManagementClient is not set.");
            }

            // We won't wait...
            deviceManagementClient.RebootSystemAsync();

            var retValue = new MethodResponse(Encoding.UTF8.GetBytes(""), 200 /*dm success code*/);
            return Task.FromResult(retValue);
        }
    }
}