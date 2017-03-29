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

        public async Task<string> GetConnectionStringAsync(uint validity = 3600)
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
