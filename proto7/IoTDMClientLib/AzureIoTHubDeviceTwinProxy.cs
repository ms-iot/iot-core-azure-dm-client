using Microsoft.Azure.Devices.Client;

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

        void IDeviceTwin.ReportProperties(string allJson)
        {
            // Will look something like this (when we get the latest SDK):
/*
            deviceClient.ReportProperties(allJson);
*/
        }
    }
}