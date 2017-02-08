using Microsoft.Azure.Devices.Client;
using Microsoft.Azure.Devices.Shared;
using Newtonsoft.Json;
using System.Collections.Generic;
using System.Text;
using System.Threading.Tasks;
using System;

namespace Microsoft.Devices.Management
{
    // This IDeviceTwin represents the actual Azure IoT Device Twin
    public class AzureIoTHubDeviceTwinProxy : IDeviceTwin
    {
        DeviceClient deviceClient;

        public AzureIoTHubDeviceTwinProxy(DeviceClient deviceClient)
        {
            this.deviceClient = deviceClient;
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

        Task IDeviceTwin.SetMethodHandlerAsync(string methodName, Func<string, Task<string>> methodHandler)
        {
            return this.deviceClient.SetMethodHandlerAsync(methodName, async (MethodRequest methodRequest, object userContext) =>
            {
                var response = await methodHandler(methodRequest.DataAsJson);
                return new MethodResponse(Encoding.UTF8.GetBytes(response), 0);
            }, null);
        }

        void IDeviceTwin.RefreshConnection()
        {
            // Recreate deviceClient using a new SAS token
        }
    }
}