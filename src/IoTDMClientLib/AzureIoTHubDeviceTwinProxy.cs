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
using Microsoft.Azure.Devices.Client;
using Microsoft.Azure.Devices.Shared;
using Newtonsoft.Json;
using System.Collections.Generic;
using System.Diagnostics;
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

        async Task<Dictionary<string, object>> IDeviceTwin.GetDesiredPropertiesAsync()
        {
            Dictionary<string, object> desiredProperties = new Dictionary<string, object>();
            Twin twin = await this.deviceClient.GetTwinAsync();
            foreach (KeyValuePair<string, object> p in twin.Properties.Desired)
            {
                desiredProperties[p.Key] = p.Value;
            }
            return desiredProperties;
        }

        async Task<string> IDeviceTwin.GetDeviceTwinPropertiesAsync()
        {
            Twin twin = await this.deviceClient.GetTwinAsync();

            StringBuilder sb = new StringBuilder();
            sb.Append("{\n");
            sb.Append("    \"properties\" : {\n");
            sb.Append("         \"desired\" : \n");
            sb.Append(twin.Properties.Desired.ToJson());
            sb.Append(",\n");
            sb.Append("         \"reported\" : \n");
            sb.Append(twin.Properties.Reported.ToJson());
            sb.Append("\n");
            sb.Append("    }\n");
            sb.Append("}\n");

            Debug.WriteLine("doc = " + sb.ToString());

            return sb.ToString();
        }

        async Task IDeviceTwin.ReportProperties(Dictionary<string, object> collection)
        {
            TwinCollection azureCollection = new TwinCollection();
            foreach (KeyValuePair<string, object> p in collection)
            {
                azureCollection[p.Key] = p.Value;
            }
            await this.deviceClient.UpdateReportedPropertiesAsync(azureCollection);
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
