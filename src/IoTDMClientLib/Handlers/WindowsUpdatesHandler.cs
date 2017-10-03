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
using Microsoft.Devices.Management.DMDataContract;
using Microsoft.Devices.Management.Message;
using System.Diagnostics;
using System.Threading.Tasks;
using Windows.Foundation.Diagnostics;

namespace Microsoft.Devices.Management
{
    class WindowsUpdatesHandler : IClientPropertyHandler
    {
        // IClientPropertyHandler
        public string PropertySectionName
        {
            get
            {
                return WindowsUpdatesDataContract.SectionName;
            }
        }

        public WindowsUpdatesHandler(
            IClientHandlerCallBack callback,
            ISystemConfiguratorProxy systemConfiguratorProxy)
        {
            _systemConfiguratorProxy = systemConfiguratorProxy;
            _deviceManagementClient = callback;
        }

        // IClientPropertyHandler
        public async Task<CommandStatus> OnDesiredPropertyChange(JToken desiredValue)
        {
            Logger.Log("WindowsUpdatesHandler.OnDesiredPropertyChange()", LoggingLevel.Verbose);

            if (!(desiredValue is JObject))
            {
                throw new Error(ErrorCodes.INVALID_DESIRED_JSON_VALUE, "Invalid json value type for the " + PropertySectionName + " node.");
            }

            WindowsUpdatesDataContract.DesiredProperties desiredProperties = WindowsUpdatesDataContract.DesiredProperties.FromJsonObject((JObject)desiredValue);

            SetWindowsUpdatesConfiguration configuration = new SetWindowsUpdatesConfiguration();
            configuration.approved = desiredProperties.approved;

            var request = new SetWindowsUpdatesRequest(configuration);
            await this._systemConfiguratorProxy.SendCommandAsync(request);

            JObject reportedProperties = await GetReportedPropertyAsync();

            Debug.WriteLine("-- Reporting Windows Updates -------------------------------------");
            Debug.WriteLine(reportedProperties.ToString());
            Debug.WriteLine("-- Reporting Windows Updates Done --------------------------------");

            // Report the updated list...
            await _deviceManagementClient.ReportPropertiesAsync(PropertySectionName, reportedProperties);

            return CommandStatus.Committed;
        }

        // IClientPropertyHandler
        public async Task<JObject> GetReportedPropertyAsync()
        {
            Logger.Log("WindowsUpdatesHandler.GetReportedPropertyAsync()", LoggingLevel.Verbose);

            var request = new Message.GetWindowsUpdatesRequest();
            var response = await _systemConfiguratorProxy.SendCommandAsync(request) as Message.GetWindowsUpdatesResponse;

            WindowsUpdatesDataContract.ReportedProperties reportedProperties = new WindowsUpdatesDataContract.ReportedProperties();

            reportedProperties.approved = response.configuration.approved;
            reportedProperties.deferUpgrade = response.configuration.deferUpgrade;
            reportedProperties.failed = response.configuration.failed;
            reportedProperties.installable = response.configuration.installable;
            reportedProperties.installed = response.configuration.installed;
            reportedProperties.lastScanTime = response.configuration.lastScanTime;
            reportedProperties.pendingReboot = response.configuration.pendingReboot;

            return reportedProperties.ToJsonObject();
        }

        private ISystemConfiguratorProxy _systemConfiguratorProxy;
        private IClientHandlerCallBack _deviceManagementClient;
    }
}
