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
// #define DEBUG_COMMPROXY_OUTPUT

using System;
using System.Diagnostics;
using System.Threading.Tasks;
using Microsoft.Devices.Management.Message;
using Windows.Foundation;
using Windows.Foundation.Diagnostics;
using Windows.Storage.Streams;
using Windows.System;
using Windows.UI.Core;

namespace Microsoft.Devices.Management
{

    // This class send requests (DMrequest) to the System Configurator and receives the responses (DMesponse) from it
    class SystemConfiguratorProxy : ISystemConfiguratorProxy
    {
        public async Task<IResponse> SendCommandAsync(IRequest command)
        {
            SystemConfiguratorProxyClientLib.SystemConfiguratorProxyClient client = 
                new SystemConfiguratorProxyClientLib.SystemConfiguratorProxyClient();
            client.Initialize();
            var response = await client.SendCommandAsync(command);
            return response;
        }

        public Task<IResponse> SendCommand(IRequest command)
        {
            SystemConfiguratorProxyClientLib.SystemConfiguratorProxyClient client =
                new SystemConfiguratorProxyClientLib.SystemConfiguratorProxyClient();
            client.Initialize();
            var response = client.SendCommand(command);
            return Task.FromResult<IResponse>(response);
        }
    }
}
