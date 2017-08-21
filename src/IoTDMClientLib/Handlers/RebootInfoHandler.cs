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

using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading.Tasks;
using Microsoft.Devices.Management.DMDataContract;
using Microsoft.Devices.Management.Message;
using Windows.Foundation.Diagnostics;

namespace Microsoft.Devices.Management
{
    class RebootInfoHandler : IClientPropertyHandler
    {
        // IClientPropertyHandler
        public string PropertySectionName
        {
            get
            {
                return RebootInfoDataContract.SectionName;
            }
        }

        public RebootInfoHandler(IClientHandlerCallBack callback,
            ISystemConfiguratorProxy systemConfiguratorProxy,
            JObject desiredCache)
        {
            _systemConfiguratorProxy = systemConfiguratorProxy;
            _deviceManagementClient = callback;
            _desiredCache = desiredCache;
        }

        private void UpdateCache(JToken desiredValue)
        {
            JToken cachedToken = _desiredCache.SelectToken(PropertySectionName);
            if (cachedToken != null)
            {
                if (cachedToken is JObject)
                {
                    JObject cachedObject = (JObject)cachedToken;
                    cachedObject.Merge(desiredValue);
                }
            }
            else
            {
                _desiredCache[PropertySectionName] = desiredValue;
            }
        }

        // IClientPropertyHandler
        public async Task<CommandStatus> OnDesiredPropertyChange(JToken desiredValue)
        {
            // Merge with existing to get values that have not changed.
            UpdateCache(desiredValue);

            JToken rebootToken = _desiredCache[PropertySectionName];
            Debug.Assert(rebootToken != null);

            if (!(rebootToken is JObject))
            {
                throw new Error(ErrorCodes.INVALID_DESIRED_JSON_VALUE, "Invalid json value type for the " + PropertySectionName + " node.");
            }

            // Parse json into an object...
            RebootInfoDataContract.DesiredProperties desiredProperties = new RebootInfoDataContract.DesiredProperties();
            desiredProperties.LoadFrom((JObject)rebootToken);

            // Construct the request and send it...
            var request = new SetRebootInfoRequest();
            request.singleRebootTime = desiredProperties.singleRebootTime;
            request.dailyRebootTime = desiredProperties.dailyRebootTime;
            await _systemConfiguratorProxy.SendCommandAsync(request);

            // Get the current state and report it...
            var currentState = await GetRebootInfoAsync();
            await _deviceManagementClient.ReportPropertiesAsync(PropertySectionName, currentState);

            return CommandStatus.Committed;
        }

        // IClientPropertyHandler
        public async Task<JObject> GetReportedPropertyAsync()
        {
            JObject currentState = await GetRebootInfoAsync();
            return await Task.Run(() => { return currentState; });
        }

        private async Task<JObject> GetRebootInfoAsync()
        {
            var request = new Message.GetRebootInfoRequest();
            var response = await _systemConfiguratorProxy.SendCommandAsync(request) as GetRebootInfoResponse;

            RebootInfoDataContract.ReportedProperties reportedProperties = new RebootInfoDataContract.ReportedProperties();
            reportedProperties.lastBootTime = response.lastBootTime;
            reportedProperties.singleRebootTime = response.singleRebootTime;
            reportedProperties.dailyRebootTime = response.dailyRebootTime;

            return JObject.FromObject(reportedProperties);
        }

        private ISystemConfiguratorProxy _systemConfiguratorProxy;
        private IClientHandlerCallBack _deviceManagementClient;
        private JObject _desiredCache;
    }
}
