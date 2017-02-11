using System;
using System.Collections.Generic;
using System.Threading.Tasks;

namespace Microsoft.Devices.Management
{
    public class OnPremDeviceTwinProxy : IDeviceTwin
    {
        public OnPremDeviceTwinProxy(/* something */)
        {
        }

        void IDeviceTwin.ReportProperties(Dictionary<string, object> collection)
        {
            // Somehow send the property to the DT
        }

        void IDeviceTwin.RefreshConnection()
        {
            // Reconnect if needed
        }

        Task IDeviceTwin.SetMethodHandlerAsync(string methodName, Func<string, Task<string>> methodHandler)
        {
            throw new NotImplementedException();
        }
    }
}