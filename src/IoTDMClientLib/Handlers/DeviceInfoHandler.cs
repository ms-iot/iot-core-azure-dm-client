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
using System.Diagnostics;
using System.Threading.Tasks;
using Windows.Foundation.Diagnostics;

namespace Microsoft.Devices.Management
{
    class DeviceInfoHandler : IClientPropertyHandler
    {
        public DeviceInfoHandler(
            IClientHandlerCallBack deviceManagementClient,
            ISystemConfiguratorProxy systemConfiguratorProxy)
        {
            _systemConfiguratorProxy = systemConfiguratorProxy;
            _deviceManagementClient = deviceManagementClient;
        }

        // IClientPropertyHandler
        public string PropertySectionName
        {
            get
            {
                return DeviceInfoDataContract.SectionName;
            }
        }

        // IClientPropertyHandler
        public async Task<CommandStatus> OnDesiredPropertyChange(JToken desiredValue)
        {
            Logger.Log("DeviceInfoHandler.OnDesiredPropertyChange()", LoggingLevel.Verbose);

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
            var request = new Message.GetDeviceInfoRequest();
            var response = await _systemConfiguratorProxy.SendCommandAsync(request) as Message.GetDeviceInfoResponse;

            DeviceInfoDataContract.ReportedProperties reportedProperties = new DeviceInfoDataContract.ReportedProperties();
            reportedProperties.batteryRuntime = response.batteryRuntime;
            reportedProperties.batteryRemaining = response.batteryRemaining;
            reportedProperties.batteryStatus = response.batteryStatus;
            reportedProperties.osEdition = response.osEdition;
            reportedProperties.secureBootState = response.secureBootState;
            reportedProperties.totalMemory = response.totalMemory;
            reportedProperties.totalStorage = response.totalStorage;
            reportedProperties.name = response.name;
            reportedProperties.processorArchitecture = response.processorArchitecture;
            reportedProperties.commercializationOperator = response.commercializationOperator;
            reportedProperties.displayResolution = response.displayResolution;
            reportedProperties.radioSwVer = response.radioSwVer;
            reportedProperties.processorType = response.processorType;
            reportedProperties.platform = response.platform;
            reportedProperties.osVer = response.osVer;
            reportedProperties.fwVer = response.fwVer;
            reportedProperties.hwVer = response.hwVer;
            reportedProperties.oem = response.oem;
            reportedProperties.type = response.type;
            reportedProperties.lang = response.lang;
            reportedProperties.dmVer = response.dmVer;
            reportedProperties.model = response.model;
            reportedProperties.manufacturer = response.manufacturer;
            reportedProperties.id = response.id;

            return JObject.FromObject(reportedProperties);
        }

        private ISystemConfiguratorProxy _systemConfiguratorProxy;
        private IClientHandlerCallBack _deviceManagementClient;
    }
}

