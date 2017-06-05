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
using Microsoft.Devices.Management.Message;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System;
using System.Diagnostics;
using System.Threading.Tasks;

namespace Microsoft.Devices.Management
{
    class WindowsUpdatePolicyHandler : SectionHandler
    {
        static string JsonSectionName = "windowsUpdatePolicy";

        public WindowsUpdatePolicyHandler(IClientHandlerCallBack callback, ISystemConfiguratorProxy systemConfiguratorProxy)
        {
            this._systemConfiguratorProxy = systemConfiguratorProxy;
            this._callback = callback;
        }

        // IClientPropertyHandler
        public override string PropertySectionName
        {
            get
            {
                return JsonSectionName; // todo: constant in data contract?
            }
        }

        protected override void ApplyProperties(JToken properties)
        {
            uint activeFields = 0;
            JObject jObject = (JObject)properties;
            foreach (JToken token in jObject.Children())
            {
                if (token is JProperty)
                {
                    JProperty jProperty = (JProperty)token;
                    switch (jProperty.Name)
                    {
                        case "activeHoursStart": activeFields |= (uint)ActiveFields.ActiveHoursStart; break;
                        case "activeHoursEnd": activeFields |= (uint)ActiveFields.ActiveHoursEnd; break;
                        case "allowAutoUpdate": activeFields |= (uint)ActiveFields.AllowAutoUpdate; break;
                        case "allowUpdateService": activeFields |= (uint)ActiveFields.AllowUpdateService; break;
                        case "branchReadinessLevel": activeFields |= (uint)ActiveFields.BranchReadinessLevel; break;
                        case "deferFeatureUpdatesPeriod": activeFields |= (uint)ActiveFields.DeferFeatureUpdatesPeriod; break;
                        case "deferQualityUpdatesPeriod": activeFields |= (uint)ActiveFields.DeferQualityUpdatesPeriod; break;
                        case "pauseFeatureUpdates": activeFields |= (uint)ActiveFields.PauseFeatureUpdates; break;
                        case "pauseQualityUpdates": activeFields |= (uint)ActiveFields.PauseQualityUpdates; break;
                        case "scheduledInstallDay": activeFields |= (uint)ActiveFields.ScheduledInstallDay; break;
                        case "scheduledInstallTime": activeFields |= (uint)ActiveFields.ScheduledInstallTime; break;
                        case "ring": activeFields |= (uint)ActiveFields.Ring; break;
                    }
                }
            }
            var configuration = JsonConvert.DeserializeObject<WindowsUpdatePolicyConfiguration>(properties.ToString());
            configuration.activeFields = activeFields;

            this._systemConfiguratorProxy.SendCommandAsync(new SetWindowsUpdatePolicyRequest(configuration));
        }

        // This will be called whenever the desired state sets this to true.
        protected override async Task ReportPropertiesAsync(bool reportProperties)
        {
            if (reportProperties)
            {
                var reportedProperties = await GetWindowsUpdatePolicyAsync();
                await this._callback.ReportPropertiesAsync(JsonSectionName, JObject.FromObject(reportedProperties));
            }
            else
            {
                await this._callback.ReportPropertiesAsync(JsonSectionName, NoReportString);
            }
        }

        // IClientPropertyHandler
        public override void OnDesiredPropertyChange(JToken desiredValue)
        {
            base.HandleDeviceTwinControlProperties(desiredValue);
        }

        // IClientPropertyHandler
        public override async Task<JObject> GetReportedPropertyAsync()
        {
            var reportedProperties = await GetWindowsUpdatePolicyAsync();
            return JObject.FromObject(reportedProperties);
        }

        public async Task<Message.WindowsUpdatePolicyConfiguration> GetWindowsUpdatePolicyAsync()
        {
            var request = new Message.GetWindowsUpdatePolicyRequest();
            Message.GetWindowsUpdatePolicyResponse response = await this._systemConfiguratorProxy.SendCommandAsync(request) as Message.GetWindowsUpdatePolicyResponse;
            return response.configuration;
        }

        private ISystemConfiguratorProxy _systemConfiguratorProxy;
        private IClientHandlerCallBack _callback;
    }
}
