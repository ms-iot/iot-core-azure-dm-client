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
    class TimeSettingsHandler : IClientPropertyHandler
    {
        const string JsonSectionName = "timeInfo";

        public TimeSettingsHandler(IClientHandlerCallBack callback, ISystemConfiguratorProxy systemConfiguratorProxy)
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
            // Default JsonConvert Deserializing changes ISO8601 date fields to "mm/dd/yyyy hh:mm:ss".
            // We need to preserve the ISO8601 since that's the format SystemConfigurator understands.
            // Because of that, we are not using:
            // Message.SetTimeInfo requestInfo = JsonConvert.DeserializeObject<Message.SetTimeInfo>(fieldsJson);

            Message.SetTimeInfoRequestData data = new Message.SetTimeInfoRequestData();

            JObject subProperties = (JObject)desiredValue;
            data.ntpServer = (string)subProperties.Property("ntpServer").Value;
            data.timeZoneBias = (int)subProperties.Property("timeZoneBias").Value;

            data.timeZoneStandardBias = (int)subProperties.Property("timeZoneStandardBias").Value;
            string standardDateString = subProperties.Property("timeZoneStandardDate").Value.ToString();
            if (!String.IsNullOrEmpty(standardDateString))
            {
                DateTime standardDate = DateTime.Parse(standardDateString).ToUniversalTime();
                data.timeZoneStandardDate = standardDate.ToString("yyyy-MM-ddTHH:mm:ssZ");
            }
            data.timeZoneStandardName = (string)subProperties.Property("timeZoneStandardName").Value;
            data.timeZoneStandardDayOfWeek = (int)subProperties.Property("timeZoneStandardDayOfWeek").Value;

            data.timeZoneDaylightBias = (int)subProperties.Property("timeZoneDaylightBias").Value;
            string daylightDateString = subProperties.Property("timeZoneDaylightDate").Value.ToString();
            if (!String.IsNullOrEmpty(daylightDateString))
            {
                DateTime daylightDate = DateTime.Parse(daylightDateString).ToUniversalTime();
                data.timeZoneDaylightDate = daylightDate.ToString("yyyy-MM-ddTHH:mm:ssZ");
            }
            data.timeZoneDaylightName = (string)subProperties.Property("timeZoneDaylightName").Value;
            data.timeZoneDaylightDayOfWeek = (int)subProperties.Property("timeZoneDaylightDayOfWeek").Value;

            Message.SetTimeInfoRequest request = new Message.SetTimeInfoRequest(data);

            await this._systemConfiguratorProxy.SendCommandAsync(request);

            var reportedProperties = await GetTimeSettingsAsync();
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
            var response = await GetTimeSettingsAsync();
            return JObject.FromObject(response.data);
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
