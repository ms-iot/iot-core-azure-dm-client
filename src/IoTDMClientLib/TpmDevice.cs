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
using Microsoft.Azure.Devices.Shared;
using Microsoft.Devices.Management.Message;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;

namespace Microsoft.Devices.Management
{
    public class TpmDevice
    {
        UInt32 logicalDeviceId = 0;
        SystemConfiguratorProxy systemConfiguratorProxy;

        public TpmDevice(UInt32 logicalDeviceId)
        {
            this.logicalDeviceId = logicalDeviceId;
            this.systemConfiguratorProxy = new SystemConfiguratorProxy();
        }

        private async Task<string> GetServiceUrlPart(int i)
        {
            var serviceUrl = await GetHeldData();
            var parts = serviceUrl.Split('/');

            if (parts.Length != 2)
            {
                throw new Exception("Unable to parse service Url in TPM");
            }

            return parts[i];
        }

        public Task<string> GetHostNameAsync() => GetServiceUrlPart(0);

        public Task<string> GetDeviceIdAsync() => GetServiceUrlPart(1);

        public async Task<string> GetSASTokenAsync(uint validity = 3600)
        {
            var result = await RunProxyAndGetResult(new Message.TpmGetSASTokenRequest(logicalDeviceId, validity));
            return (result as Message.StringResponse).Response;
        }

        public async Task<string> GetConnectionString(uint validity = 3600)
        {
            string deviceId = await GetDeviceIdAsync();
            string hostName = await GetHostNameAsync();
            string sasToken = await GetSASTokenAsync(validity);
            string connectionString = "";
            if ((hostName.Length > 0) && (deviceId.Length > 0) && (sasToken.Length > 0))
            {
                connectionString = "HostName=" + hostName + ";DeviceId=" + deviceId + ";SharedAccessSignature=" + sasToken;
            }
            return connectionString;
        }

        string heldData = "";

        private async Task<string> GetHeldData()
        {
            if (heldData == string.Empty)
            {
                // Not retrieved yet, retrieve
                var result = await RunProxyAndGetResult(new Message.TpmGetServiceUrlRequest(logicalDeviceId));
                heldData = (result as Message.StringResponse).Response;
            }
            return heldData;
        }

        private async Task<IResponse> RunProxyAndGetResult(IRequest request)
        {
            return await this.systemConfiguratorProxy.SendCommandAsync(request);
        }

    }
}
