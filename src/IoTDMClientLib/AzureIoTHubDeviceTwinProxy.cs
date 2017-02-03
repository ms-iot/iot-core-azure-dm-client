using Microsoft.Azure.Devices.Client;
using Microsoft.Azure.Devices.Shared;
using Newtonsoft.Json;
using Microsoft.Devices.Management.Message;
using System.Collections.Generic;
using System.Text;
using System.Threading.Tasks;
using Windows.Data.Json;

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

        void IDeviceTwin.SetMethodHandlerAsync(string methodName, Func<string, Task<string>> methodHandler)
        {
            this.deviceClient.SetMethodHandler(methodName, async (MethodRequest methodRequest, object userContext) =>
            {
                var response = await methodHandler(methodRequest.DataAsJson);
                return new MethodResponse(Encoding.UTF8.GetBytes(response), 0);
            }, null);
        }

        void IDeviceTwin.RefreshConnection()
        {
            // Recreate deviceClient using a new SAS token
        }

        Task<MethodResponse> InstallAppAsync(MethodRequest methodRequest, object userContext)
        {
            if (deviceManagementClient == null)
            {
                throw new System.Exception("ManagementClient is not set.");
            }

            var requestJson = JsonObject.Parse(methodRequest.DataAsJson);
            AppInstallInfo installInfo;

            // Dependencies
            {
                var dependenciesJson = requestJson.GetNamedArray("Dependencies");
                for (int i = 0; i < dependenciesJson.Count; i++)
                {
                    JsonObject dependencyJson = dependenciesJson.GetObjectAt(i);
                    var dependencyInfo = new AzureFileTransferInfo() { ConnectionString = dependencyJson.GetNamedString("ConnectionString"), ContainerName = dependencyJson.GetNamedString("ContainerName"), BlobName = dependencyJson.GetNamedString("BlobName"), Upload = false };
                    var transferTask = deviceManagementClient.TransferFileAsync(dependencyInfo);
                    transferTask.Wait();

                    
                }
            }

            // Cert
            {
                var certJson = requestJson.GetNamedObject("Cert");
                var certInfo = new AzureFileTransferInfo() { ConnectionString = certJson.GetNamedString("ConnectionString"), ContainerName = certJson.GetNamedString("ContainerName"), BlobName = certJson.GetNamedString("BlobName"), Upload = false };
                var transferTask = deviceManagementClient.TransferFileAsync(certInfo);
                transferTask.Wait();
            }

            // Appx
            {
                var appxJson = requestJson.GetNamedObject("Appx");
                var appxInfo = new AzureFileTransferInfo() { ConnectionString = appxJson.GetNamedString("ConnectionString"), ContainerName = appxJson.GetNamedString("ContainerName"), BlobName = appxJson.GetNamedString("BlobName"), Upload = false };
                var transferTask = deviceManagementClient.TransferFileAsync(appxInfo);
                transferTask.Wait();
            }


            var t = deviceManagementClient.InstallAppAsync(appInstallInfo);

            var retValue = new MethodResponse(Encoding.UTF8.GetBytes(""), 0 /*(int)t.Result.returnCode*/);
            return Task.FromResult(retValue);
        }
    }
}