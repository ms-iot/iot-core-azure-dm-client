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

using Newtonsoft.Json.Linq;
using System;
using System.Threading.Tasks;

namespace Microsoft.Devices.Management
{
    class TimeServiceHandler : IClientPropertyHandler
    {
        const string JsonSectionName = "timeService";
        const string JsonEnabled = "enabled";
        const string JsonStartup = "startup";
        const string JsonStarted = "started";

        public TimeServiceHandler(IClientHandlerCallBack callback, ISystemConfiguratorProxy systemConfiguratorProxy)
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
            Message.TimeServiceData data = new Message.TimeServiceData();

            JObject subProperties = (JObject)desiredValue;

            data.enabled = subProperties.Property(JsonEnabled).Value.ToString();
            data.startup = subProperties.Property(JsonStartup).Value.ToString();
            data.started = subProperties.Property(JsonStarted).Value.ToString();

            var request = new Message.SetTimeServiceRequest(data);

            await this._systemConfiguratorProxy.SendCommandAsync(request);

            var reportedProperties = await GetTimeServiceAsync();
            await this._callback.ReportPropertiesAsync(JsonSectionName, JObject.FromObject(reportedProperties.data));
        }

        // IClientPropertyHandler
        public async Task<DesiredPropertyApplication> OnDesiredPropertyChange(JToken desiredValue)
        {
            await HandleDesiredPropertyChangeAsync(desiredValue);

            return DesiredPropertyApplication.Continue;
        }

        // IClientPropertyHandler
        public async Task<JObject> GetReportedPropertyAsync()
        {
            var response = await GetTimeServiceAsync();
            return JObject.FromObject(response.data);
        }

        public async Task<Message.GetTimeServiceResponse> GetTimeServiceAsync()
        {
            var request = new Message.GetTimeServiceRequest();
            var response = await this._systemConfiguratorProxy.SendCommandAsync(request);
            return response as Message.GetTimeServiceResponse;
        }

        private ISystemConfiguratorProxy _systemConfiguratorProxy;
        private IClientHandlerCallBack _callback;
    }
}

