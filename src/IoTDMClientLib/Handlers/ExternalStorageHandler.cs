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
    class ExternalStorageHandler : IClientPropertyHandler
    {
        // IClientPropertyHandler
        public string PropertySectionName
        {
            get
            {
                return ExternalStorageDataContract.SectionName;
            }
        }

        public ExternalStorageHandler(IClientHandlerCallBack callback, ISystemConfiguratorProxy systemConfiguratorProxy)
        {
            this._systemConfiguratorProxy = systemConfiguratorProxy;
            this._callback = callback;
        }

        // IClientPropertyHandler
        public async Task<CommandStatus> OnDesiredPropertyChange(JToken desiredValue)
        {
            if (!(desiredValue is JObject))
            {
                throw new Error(ErrorCodes.INVALID_DESIRED_JSON_VALUE, "Invalid json value type for the " + PropertySectionName + " node.");
            }

            ExternalStorageDataContract.DesiredProperties desiredProperties = new ExternalStorageDataContract.DesiredProperties();
            desiredProperties.LoadFrom((JObject)desiredValue);

            ConnectionString = desiredProperties.connectionString;

            // Report to the device twin....
            var reportedProperties = await GetReportedPropertyAsync();
            await _callback.ReportPropertiesAsync(PropertySectionName, reportedProperties);

            return CommandStatus.Committed;
        }

        // IClientPropertyHandler
        public Task<JObject> GetReportedPropertyAsync()
        {
            ExternalStorageDataContract.ReportedProperties reportedProperties = new ExternalStorageDataContract.ReportedProperties();
            reportedProperties.connectionString = ConnectionString;
            return Task.FromResult<JObject>(JObject.FromObject(reportedProperties));
        }

        public string ConnectionString { get; private set; }

        private ISystemConfiguratorProxy _systemConfiguratorProxy;
        private IClientHandlerCallBack _callback;
    }
}
