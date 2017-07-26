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
using System.Collections.Generic;
using System;
using System.Threading.Tasks;

namespace Microsoft.Devices.Management
{
    // This interface allows the client to send reported properties
    // to an unspecified DeviceTwin. The abstraction allows us to substitute different
    // DT implementations, such as DT in Azure or DT on prem.
    // The DM only needs an ability to send reported properties to DT
    public interface IDeviceTwin
    {
        Task<string> GetDeviceTwinPropertiesAsync();

        Task<Dictionary<string, object>> GetDesiredPropertiesAsync();

        Task ReportProperties(Dictionary<string, object> collection);

        Task SetMethodHandlerAsync(string methodName, Func<string, Task<string>> methodHandler);

        // This API is called when the connection has expired (such as when SAS token has expired)
        void RefreshConnection();

        Task SendMessageAsync(string message, IDictionary<string, string> properties);
    }
}
