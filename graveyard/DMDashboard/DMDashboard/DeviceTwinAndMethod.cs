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
//
// Copied from the Azure SDK DeviceExplorer sample.
//
using Newtonsoft.Json;
using Microsoft.Azure.Devices;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;


namespace DMDashboard
{
    struct DeviceMethodReturnValue
    {
        public string Status;
        public string Payload;
    }

    struct DeviceTwinData
    {
        public string deviceJson;
        public string tagsJson;
        public string reportedPropertiesJson;
        public string desiredPropertiesJson;
    }

    class DeviceTwinAndMethod
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
                MessageBox.Show(ex.Message + Environment.NewLine + "Make sure you are using the latest Microsoft.Azure.Devices package.", "Device Twin Properties");
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
                        MessageBox.Show(messageDeviceTwinFunctionalityNotFound, "Device Twin Properties Update");
                    }
                }
                catch (Exception ex)
                {
                    string errMess = "Update Twin failed. Exception: " + ex.ToString();
                    MessageBox.Show(errMess, "Device Twin Desired Properties Update");
                }
            }
            else
            {
                MessageBox.Show("Registry Manager is no initialized!", "Device Twin Desired Properties Update");
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

                deviceMethodReturnValue.Status = result.Status;
                deviceMethodReturnValue.Payload = result.GetPayloadAsJson();

                isOK = true;
            }
            catch (Exception ex)
            {
                exStr = ex.Message;
            }

            if (!isOK)
            {
                MessageBox.Show(exStr, "Device Twin Properties");
            }
            return deviceMethodReturnValue;
        }
    }
}
