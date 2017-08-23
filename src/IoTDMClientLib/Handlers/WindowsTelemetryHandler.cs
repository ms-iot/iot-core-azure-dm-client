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
using System.Threading.Tasks;

namespace Microsoft.Devices.Management
{
    public enum WindowsTelemetryLevel
    {
        Security,
        Basic,
        Enhanced,
        Full
    }

    class WindowsTelemetryHandler : IClientPropertyHandler
    {
        public WindowsTelemetryHandler(IClientHandlerCallBack callback, ISystemConfiguratorProxy systemConfiguratorProxy)
        {
            this._systemConfiguratorProxy = systemConfiguratorProxy;
            this._callback = callback;
        }

        // IClientPropertyHandler
        public string PropertySectionName
        {
            get
            {
                return WindowsTelemetryDataContract.SectionName;
            }
        }

        // IClientPropertyHandler
        public async Task<CommandStatus> OnDesiredPropertyChange(JToken desiredValue)
        {
            if (!(desiredValue is JObject))
            {
                throw new Error(ErrorCodes.INVALID_DESIRED_JSON_VALUE, "Invalid json value type for the " + PropertySectionName + " node.");
            }

            WindowsTelemetryDataContract.DesiredProperties desiredProperties = new WindowsTelemetryDataContract.DesiredProperties();
            desiredProperties.LoadFrom((JObject)desiredValue);

            await SetLevelAsync(desiredProperties.level);

            return CommandStatus.Committed;
        }

        // IClientPropertyHandler
        public async Task<JObject> GetReportedPropertyAsync()
        {
            var request = new Message.GetWindowsTelemetryRequest();
            var response = await _systemConfiguratorProxy.SendCommandAsync(request) as Message.GetWindowsTelemetryResponse;

            WindowsTelemetryDataContract.ReportedProperties reportedProperties = new WindowsTelemetryDataContract.ReportedProperties();
            reportedProperties.level = response.data.level;
            return JObject.FromObject(reportedProperties);
        }

        public async Task<WindowsTelemetryLevel> GetLevelAsync()
        {
            var request = new Message.GetWindowsTelemetryRequest();
            var response = await _systemConfiguratorProxy.SendCommandAsync(request) as Message.GetWindowsTelemetryResponse;

            WindowsTelemetryLevel level = WindowsTelemetryLevel.Enhanced;

            switch (response.data.level)
            {
                case WindowsTelemetryDataContract.JsonSecurity:
                    level = WindowsTelemetryLevel.Security;
                    break;
                case WindowsTelemetryDataContract.JsonBasic:
                    level = WindowsTelemetryLevel.Basic;
                    break;
                case WindowsTelemetryDataContract.JsonEnhanced:
                    level = WindowsTelemetryLevel.Enhanced;
                    break;
                case WindowsTelemetryDataContract.JsonFull:
                    level = WindowsTelemetryLevel.Full;
                    break;
                default:
                    throw new Error(ErrorCodes.INVALID_WINDOWS_TELEMETRY_LEVEL, "Found: " + response.data.level);
            }

            return level;
        }

        public async Task SetLevelAsync(WindowsTelemetryLevel level)
        {
            string levelString = WindowsTelemetryDataContract.JsonEnhanced;
            switch (level)
            {
                case WindowsTelemetryLevel.Security:
                    levelString = WindowsTelemetryDataContract.JsonSecurity;
                    break;
                case WindowsTelemetryLevel.Basic:
                    levelString = WindowsTelemetryDataContract.JsonBasic;
                    break;
                case WindowsTelemetryLevel.Enhanced:
                    levelString = WindowsTelemetryDataContract.JsonEnhanced;
                    break;
                case WindowsTelemetryLevel.Full:
                    levelString = WindowsTelemetryDataContract.JsonFull;
                    break;
            }

            await SetLevelAsync(levelString);
        }

        private async Task SetLevelAsync(string levelString)
        {
            Message.WindowsTelemetryData data = new Message.WindowsTelemetryData();
            data.level = levelString;

            // Construct the request and send it...
            var request = new Message.SetWindowsTelemetryRequest(data);
            await this._systemConfiguratorProxy.SendCommandAsync(request);

            // Report to the device twin....
            var reportedProperties = await GetReportedPropertyAsync();
            await _callback.ReportPropertiesAsync(PropertySectionName, reportedProperties);
        }

        private ISystemConfiguratorProxy _systemConfiguratorProxy;
        private IClientHandlerCallBack _callback;
    }
}

