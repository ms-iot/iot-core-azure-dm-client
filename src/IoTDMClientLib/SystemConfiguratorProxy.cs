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
        SystemConfiguratorProxyClient.SCProxyClient _client;
        public SystemConfiguratorProxy()
        {
            _client = new SystemConfiguratorProxyClient.SCProxyClient();
            var result = _client.Initialize();
            if (0 != result)
            {
                throw new Error((int)result, "SystemConfiguratorProxyClient failed to initialize, be sure that SystemConfigurator is running.");
            }
        }

        private void ThrowError(IResponse response)
        {
            if (response == null)
            {
                throw new Error(ErrorSubSystem.Unknown, -1, "SystemConfigurator returned a null response.");
            }
            else if (response is ErrorResponse)
            {
                var errorResponse = response as ErrorResponse;
                string message = "Sub-system=" + errorResponse.SubSystem.ToString() + ", code=" + errorResponse.ErrorCode + ", messag=" + errorResponse.ErrorMessage;
                Logger.Log(message, LoggingLevel.Error);
                Debug.WriteLine(message);
                throw new Error(errorResponse.SubSystem, errorResponse.ErrorCode, errorResponse.ErrorMessage);
            }
            else if (response is StringResponse)
            {
                var stringResponse = response as StringResponse;
                string message = "Error Tag(" + stringResponse.Tag.ToString() + ") : " + stringResponse.Status.ToString() + " : " + stringResponse.Response;
                Logger.Log(message, LoggingLevel.Error);
                Debug.WriteLine(message);
                throw new Error(ErrorSubSystem.Unknown, -1, message);
            }
        }

        public async Task<IResponse> SendCommandAsync(IRequest command)
        {
            var response = await _client.SendCommandAsync(command);
            if (response.Status != ResponseStatus.Success)
            {
                ThrowError(response);
            }
            return response;
        }

        public Task<IResponse> SendCommand(IRequest command)
        {
            var response = _client.SendCommand(command);
            if (response.Status != ResponseStatus.Success)
            {
                ThrowError(response);
            }
            return Task.FromResult<IResponse>(response);
        }
    }
}
