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

using DMDataContract;
using Microsoft.Devices.Management.Message;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System.Diagnostics;
using System.Threading.Tasks;

namespace Microsoft.Devices.Management
{
    class WindowsUpdatePolicyHandler : IClientPropertyHandler
    {
        static string JsonSectionName = "windowsUpdatePolicy";

        public WindowsUpdatePolicyHandler(IClientHandlerCallBack callback, ISystemConfiguratorProxy systemConfiguratorProxy)
        {
            this._systemConfiguratorProxy = systemConfiguratorProxy;
            this._callback = callback;
        }

        // IClientPropertyHandler
        public string PropertySectionName
        {
            get
            {
                return JsonSectionName; // todo: constant in data contract?
            }
        }

        private async Task HandleDesiredPropertyChangeAsync(JToken desiredValue)
        {
            Debug.WriteLine("WindowsUpdatePolicy:");
            Debug.WriteLine(desiredValue.ToString());

            Message.SetWindowsUpdatePolicyRequest request = Message.SetWindowsUpdatePolicyRequest.Deserialize(desiredValue.ToString());

            // Always send down to SystemConfigurator because we need to persist the reporting (if specified).
            await this._systemConfiguratorProxy.SendCommandAsync(request);

            Message.GetWindowsUpdatePolicyResponse reportedProperties = await GetWindowsUpdatePolicyAsync();
            if (reportedProperties.ReportToDeviceTwin == DMJSonConstants.YesString)
            {
                await this._callback.ReportPropertiesAsync(JsonSectionName, JObject.FromObject(reportedProperties.data));
            }
            else
            {
                await this._callback.ReportPropertiesAsync(JsonSectionName, DMJSonConstants.NoString);
            }
        }

        // IClientPropertyHandler
        public void OnDesiredPropertyChange(JToken desiredValue)
        {
            HandleDesiredPropertyChangeAsync(desiredValue);
        }

        // IClientPropertyHandler
        public async Task<JObject> GetReportedPropertyAsync()
        {
            Message.GetWindowsUpdatePolicyResponse reportedProperties = await GetWindowsUpdatePolicyAsync();
            return JObject.FromObject(reportedProperties);
        }

        public async Task<Message.GetWindowsUpdatePolicyResponse> GetWindowsUpdatePolicyAsync()
        {
            var request = new Message.GetWindowsUpdatePolicyRequest();
            Message.GetWindowsUpdatePolicyResponse response = await this._systemConfiguratorProxy.SendCommandAsync(request) as Message.GetWindowsUpdatePolicyResponse;
            return response;
        }

        private ISystemConfiguratorProxy _systemConfiguratorProxy;
        private IClientHandlerCallBack _callback;
    }
}
