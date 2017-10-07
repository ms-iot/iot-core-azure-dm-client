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

using Microsoft.Devices.Management.DMDataContract;
using Newtonsoft.Json.Linq;
using System.Threading.Tasks;

namespace Microsoft.Devices.Management
{
    class TimeSettingsHandler : IClientPropertyHandler
    {
        public TimeSettingsHandler(IClientHandlerCallBack callback, ISystemConfiguratorProxy systemConfiguratorProxy)
        {
            _systemConfiguratorProxy = systemConfiguratorProxy;
            _callback = callback;
        }

        // IClientPropertyHandler
        public string PropertySectionName
        {
            get
            {
                return TimeSettingsDataContract.SectionName;
            }
        }

        private async Task HandleDesiredPropertyChangeAsync(JToken desiredValue)
        {
            if (!(desiredValue is JObject))
            {
                throw new Error(ErrorCodes.INVALID_DESIRED_JSON_VALUE, "Invalid json value type for the " + PropertySectionName + " node.");
            }

            TimeSettingsDataContract.DesiredProperties desiredProperties = TimeSettingsDataContract.DesiredProperties.FromJsonObject((JObject)desiredValue);

            Message.SetTimeInfoRequestData data = new Message.SetTimeInfoRequestData();

            data.ntpServer = desiredProperties.ntpServer;

            // Use registry settings?
            data.dynamicDaylightTimeDisabled = desiredProperties.dynamicDaylightTimeDisabled;

            // Option 1 (if dynamicDaylightTimeDisabled = false)
            data.timeZoneKeyName = desiredProperties.timeZoneKeyName;

            // Option 2 (if dynamicDaylightTimeDisabled = true || timeZoneKeyName is not found)
            data.timeZoneBias = desiredProperties.timeZoneBias;
            data.timeZoneStandardBias = desiredProperties.timeZoneStandardBias;
            data.timeZoneStandardDate = desiredProperties.timeZoneStandardDate;
            data.timeZoneStandardDayOfWeek = desiredProperties.timeZoneStandardDayOfWeek;
            data.timeZoneStandardName = desiredProperties.timeZoneStandardName;
            data.timeZoneDaylightBias = desiredProperties.timeZoneDaylightBias;
            data.timeZoneDaylightDate = desiredProperties.timeZoneDaylightDate;
            data.timeZoneDaylightDayOfWeek = desiredProperties.timeZoneDaylightDayOfWeek;
            data.timeZoneDaylightName = desiredProperties.timeZoneDaylightName;

            await _systemConfiguratorProxy.SendCommandAsync(new Message.SetTimeInfoRequest(data));

            var reportedProperties = await GetTimeSettingsAsync();
            await _callback.ReportPropertiesAsync(PropertySectionName, JObject.FromObject(reportedProperties.data));
        }

        // IClientPropertyHandler
        public async Task<CommandStatus> OnDesiredPropertyChange(JToken desiredValue)
        {
            await HandleDesiredPropertyChangeAsync(desiredValue);

            return CommandStatus.Committed;
        }

        // IClientPropertyHandler
        public async Task<JObject> GetReportedPropertyAsync()
        {
            var response = await GetTimeSettingsAsync();

            TimeSettingsDataContract.ReportedProperties reportedProperties = new TimeSettingsDataContract.ReportedProperties();

            reportedProperties.localTime = response.data.localTime;
            reportedProperties.ntpServer = response.data.ntpServer;
            reportedProperties.dynamicDaylightTimeDisabled = response.data.dynamicDaylightTimeDisabled;
            reportedProperties.timeZoneKeyName = response.data.timeZoneKeyName;
            reportedProperties.timeZoneBias = response.data.timeZoneBias;
            reportedProperties.timeZoneStandardBias = response.data.timeZoneStandardBias;
            reportedProperties.timeZoneStandardDate = response.data.timeZoneStandardDate;
            reportedProperties.timeZoneStandardDayOfWeek = response.data.timeZoneStandardDayOfWeek;
            reportedProperties.timeZoneStandardName = response.data.timeZoneStandardName;
            reportedProperties.timeZoneDaylightBias = response.data.timeZoneDaylightBias;
            reportedProperties.timeZoneDaylightDate = response.data.timeZoneDaylightDate;
            reportedProperties.timeZoneDaylightDayOfWeek = response.data.timeZoneDaylightDayOfWeek;
            reportedProperties.timeZoneDaylightName = response.data.timeZoneDaylightName;

            return reportedProperties.ToJsonObject();
        }

        public async Task<Message.GetTimeInfoResponse> GetTimeSettingsAsync()
        {
            var request = new Message.GetTimeInfoRequest();
            var response = await this._systemConfiguratorProxy.SendCommandAsync(request);
            return response as Message.GetTimeInfoResponse;
        }

        private ISystemConfiguratorProxy _systemConfiguratorProxy;
        private IClientHandlerCallBack _callback;
    }
}
