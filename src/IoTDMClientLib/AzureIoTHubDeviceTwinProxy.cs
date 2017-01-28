using Microsoft.Azure.Devices.Client;
using Microsoft.Azure.Devices.Shared;
using Newtonsoft.Json;
using System.Collections.Generic;
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
            this.deviceClient.SetMethodHandler("microsoft.management.immediateReboot", ImmediateRebootAsync, null);
        }

        public void SetManagementClient(DeviceManagementClient deviceManagementClient)
        {
            this.deviceManagementClient = deviceManagementClient;
        }

        void IDeviceTwin.ReportProperties(Dictionary<string, object> collection)
        {
            TwinCollection azureCollection = new TwinCollection();
            foreach (KeyValuePair<string, object> p in collection)
            {
                azureCollection[p.Key] = p.Value;
            }
            this.deviceClient.UpdateReportedPropertiesAsync(azureCollection);
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

            Task<DeviceManagementClient.DMMethodResult> t = deviceManagementClient.ReportAllPropertiesAsync();
            // t.Wait();    // ToDo: Investigate why this causes a deadlock.

            var retValue = new MethodResponse(Encoding.UTF8.GetBytes(""), 0 /*(int)t.Result.returnCode*/);
            return Task.FromResult(retValue);
        }

        Task<MethodResponse> DoFactoryResetAsync(MethodRequest methodRequest, object userContext)
        {
            if (deviceManagementClient == null)
            {
                throw new System.Exception("ManagementClient is not set.");
            }

#if false // TODO
            Task<DeviceManagementClient.DMMethodResult> t = deviceManagementClient.DoFactoryResetAsync();
            // t.Wait();    // ToDo: Investigate why this causes a deadlock.
#endif

            var retValue = new MethodResponse(Encoding.UTF8.GetBytes(""), 0 /*(int)t.Result.returnCode*/);
            return Task.FromResult(retValue);
        }

        Task<MethodResponse> ImmediateRebootAsync(MethodRequest methodRequest, object userContext)
        {
            // Start the reboot operation asynchrnously, which may or may not succeed
            var rebootOp = deviceManagementClient.ImmediateRebootAsync();

            // TODO: consult the active hours schedule to make sure reboot is allowed
            var rebootAllowed = true;

            var response = JsonConvert.SerializeObject(new { response = rebootAllowed ? "accepted" : "rejected" });

            var retValue = new MethodResponse(Encoding.UTF8.GetBytes(response), 0);
            return Task.FromResult(retValue);
        }
    }
}