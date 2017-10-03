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
    public enum ServiceStartup
    {
        Manual,
        Auto
    }

    public class TimeServiceState
    {
        public bool enabled;
        public ServiceStartup startup;
        public bool started;
        public SettingsPriority settingsPriority;
    }

    static class TimeServiceHandlerHelpers
    {
        public static TimeServiceState AsState(this TimeServiceDataContract.ReportedProperties reportedProperties)
        {
            TimeServiceState state = new TimeServiceState();
            state.enabled = reportedProperties.enabled == TimeServiceDataContract.JsonYes;
            state.startup = reportedProperties.startup == TimeServiceDataContract.JsonAuto ? ServiceStartup.Auto : ServiceStartup.Manual;
            state.started = reportedProperties.started == TimeServiceDataContract.JsonYes;
            state.settingsPriority = PolicyHelpers.SettingsPriorityFromString(reportedProperties.sourcePriority);
            return state;
        }
    }

    class TimeServiceHandler : IClientPropertyHandler
    {
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
                return TimeServiceDataContract.SectionName;
            }
        }

        // IClientPropertyHandler
        public async Task<CommandStatus> OnDesiredPropertyChange(JToken desiredValue)
        {
            if (!(desiredValue is JObject))
            {
                throw new Error(ErrorCodes.INVALID_DESIRED_JSON_VALUE, "Invalid json value type for the " + PropertySectionName + " node.");
            }

            TimeServiceDataContract.DesiredProperties desiredProperties = new TimeServiceDataContract.DesiredProperties();
            desiredProperties.LoadFrom((JObject)desiredValue);

            // Construct the request and send it...
            Message.Policy policy = new Message.Policy();
            policy.source = Message.PolicySource.Remote;
            policy.sourcePriorities = desiredProperties.sourcePriority == PolicyDataContract.JsonLocal ? _priorityLocal : _priorityRemote;

            Message.TimeServiceData data = new Message.TimeServiceData();
            data.enabled = desiredProperties.enabled;
            data.startup = desiredProperties.startup;
            data.started = desiredProperties.started;
            data.policy = policy;

            var request = new Message.SetTimeServiceRequest(data);
            await this._systemConfiguratorProxy.SendCommandAsync(request);

            // Report to the device twin....
            var reportedProperties = await GetTimeServiceAsync();
            await this._callback.ReportPropertiesAsync(PropertySectionName, JObject.FromObject(reportedProperties));

            return CommandStatus.Committed;
        }

        // IClientPropertyHandler
        public async Task<JObject> GetReportedPropertyAsync()
        {
            var response = await GetTimeServiceAsync();
            return JObject.FromObject(response);
        }

        public async Task SetTimeServiceAsync(TimeServiceState userDesiredState)
        {
            // Construct the request and send it...
            Message.Policy policy = new Message.Policy();
            policy.source = Message.PolicySource.Local;
            policy.sourcePriorities = userDesiredState.settingsPriority == SettingsPriority.Local ? _priorityLocal : _priorityRemote;

            Message.TimeServiceData data = new Message.TimeServiceData();
            data.enabled = userDesiredState.enabled ? TimeServiceDataContract.JsonYes : TimeServiceDataContract.JsonNo;
            data.startup = userDesiredState.startup == ServiceStartup.Auto ? TimeServiceDataContract.JsonAuto : TimeServiceDataContract.JsonManual;
            data.started = userDesiredState.started ? TimeServiceDataContract.JsonYes : TimeServiceDataContract.JsonNo;
            data.policy = policy;

            var setRequest = new Message.SetTimeServiceRequest(data);
            await this._systemConfiguratorProxy.SendCommandAsync(setRequest);

            // Get the current state....
            TimeServiceDataContract.ReportedProperties reportedProperties = await GetTimeServiceAsync();
            await this._callback.ReportPropertiesAsync(PropertySectionName, JObject.FromObject(reportedProperties));
        }

        private async Task<TimeServiceDataContract.ReportedProperties> GetTimeServiceAsync()
        {
            // Get the current state....
            var getRequest = new Message.GetTimeServiceRequest();
            var response = await this._systemConfiguratorProxy.SendCommandAsync(getRequest) as Message.GetTimeServiceResponse;

            // Report it to the device twin...
            TimeServiceDataContract.ReportedProperties reportedProperties = new TimeServiceDataContract.ReportedProperties();
            reportedProperties.started = response.data.started;
            reportedProperties.startup = response.data.startup;
            reportedProperties.enabled = response.data.enabled;
            reportedProperties.sourcePriority = PolicyHelpers.SourcePriorityFromPolicy(response.data.policy);

            return reportedProperties;
        }

        public async Task<TimeServiceState> GetTimeServiceStateAsync()
        {
            TimeServiceDataContract.ReportedProperties reportedProperties = await GetTimeServiceAsync();
            return reportedProperties.AsState();
        }

        private readonly Message.PolicySource[] _priorityLocal = {
            Message.PolicySource.Local, // local takes precedence
            Message.PolicySource.Remote
        };

        private readonly Message.PolicySource[] _priorityRemote = {
            Message.PolicySource.Remote, // remote takes precedence
            Message.PolicySource.Local
        };

        private ISystemConfiguratorProxy _systemConfiguratorProxy;
        private IClientHandlerCallBack _callback;
    }
}

