using System;

namespace Microsoft.Devices.Management
{
    public class OnPremDeviceTwinProxy : IDeviceTwin
    {
        public OnPremDeviceTwinProxy(/* something */)
        {
        }

        void IDeviceTwin.ReportProperties(string allJson)
        {
            // Somehow send the property to the DT
        }
    }
}