using Microsoft.Azure.Devices;
using Newtonsoft.Json;
using System;
using System.Diagnostics;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;

namespace DMController
{
    public struct DeviceMethodReturnValue
    {
        public string Status;
        public string Payload;
    }

    public struct DeviceTwinData
    {
        public string deviceJson;
        public string tagsJson;
        public string reportedPropertiesJson;
        public string desiredPropertiesJson;
    }

    public class DeviceTwinAndMethod
    {
        private string messageDeviceTwinFunctionalityNotFound = "Device Twin functionality not found." + Environment.NewLine + "Make sure you are using the latest Microsoft.Azure.Devices package.";
        private string connString;
        private string deviceName;

        public DeviceTwinAndMethod(string connectionString, string deviceId)
        {
            connString = connectionString;
            deviceName = deviceId;
        }

        public async Task<DeviceTwinData> GetDeviceTwinData()
        {
            DeviceTwinData result = new DeviceTwinData();

            dynamic registryManager = RegistryManager.CreateFromConnectionString(connString);
            try
            {
                var deviceTwin = await registryManager.GetTwinAsync(deviceName);
                if (deviceTwin != null)
                {
                    result.deviceJson = deviceTwin.ToJson();
                    result.tagsJson = deviceTwin.Tags.ToJson();
                    result.reportedPropertiesJson = deviceTwin.Properties.Reported.ToJson();
                    result.desiredPropertiesJson = deviceTwin.Properties.Desired.ToJson();
                }
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message + Environment.NewLine + "Make sure you are using the latest Microsoft.Azure.Devices package.", "Device Twin Properties");
            }
            return result;
        }

        public async Task<bool> UpdateTwinData(string updateJson)
        {
            dynamic registryManager = RegistryManager.CreateFromConnectionString(connString);
            if (registryManager != null)
            {
                try
                {
                    string assemblyClassName = "Twin";
                    Type typeFound = (from assembly in AppDomain.CurrentDomain.GetAssemblies()
                                      from assemblyType in assembly.GetTypes()
                                      where assemblyType.Name == assemblyClassName
                                      select assemblyType).FirstOrDefault();

                    if (typeFound != null)
                    {
                        var deviceTwin = await registryManager.GetTwinAsync(deviceName);
                        dynamic dp = JsonConvert.DeserializeObject(updateJson, typeFound);
                        dp.DeviceId = deviceName;
                        dp.ETag = deviceTwin.ETag;
                        registryManager.UpdateTwinAsync(dp.DeviceId, dp, dp.ETag);
                    }
                    else
                    {
                        Debug.WriteLine(messageDeviceTwinFunctionalityNotFound, "Device Twin Properties Update");
                    }
                }
                catch (Exception ex)
                {
                    string errMess = "Update Twin failed. Exception: " + ex.ToString();
                    Debug.WriteLine(errMess, "Device Twin Desired Properties Update");
                }
            }
            else
            {
                Debug.WriteLine("Registry Manager is no initialized!", "Device Twin Desired Properties Update");
            }
            await Task.Delay(1000);
            return true; ;
        }

        public async Task<DeviceMethodReturnValue> CallDeviceMethod(string deviceMethodName, string deviceMethodPayload, TimeSpan timeoutInSeconds, CancellationToken cancellationToken)
        {
            bool isOK = false;
            string exStr = "";
            DeviceMethodReturnValue deviceMethodReturnValue;
            deviceMethodReturnValue.Status = "";
            deviceMethodReturnValue.Payload = "";

            var serviceClient = ServiceClient.CreateFromConnectionString(connString);
            try
            {
                var cloudToDeviceMethod = new CloudToDeviceMethod(deviceMethodName, timeoutInSeconds);
                cloudToDeviceMethod.SetPayloadJson(deviceMethodPayload);

                var result = await serviceClient.InvokeDeviceMethodAsync(deviceName, cloudToDeviceMethod, cancellationToken);

                deviceMethodReturnValue.Status = result.Status.ToString();
                deviceMethodReturnValue.Payload = result.GetPayloadAsJson();

                isOK = true;
            }
            catch (Exception ex)
            {
                exStr = ex.Message;
            }

            if (!isOK)
            {
                Debug.WriteLine(exStr, "Device Twin Properties");
            }
            return deviceMethodReturnValue;
        }
    }
}
