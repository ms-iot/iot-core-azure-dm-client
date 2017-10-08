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
using System;
using System.Threading.Tasks;

namespace Microsoft.Devices.Management
{
    public enum WindowsUpdateRing
    {
        EarlyAdopter,
        Preview,
        GeneralAvailability
    }

    public class WindowsUpdateRingState
    {
        public WindowsUpdateRing ring;
        public SettingsPriority settingsPriority;

        public static string RingToJsonString(WindowsUpdateRing ring)
        {
            switch (ring)
            {
                case WindowsUpdateRing.EarlyAdopter:
                    return WindowsUpdatePolicyDataContract.JsonEarlyAdopter;
                case WindowsUpdateRing.Preview:
                    return WindowsUpdatePolicyDataContract.JsonPreview;
                case WindowsUpdateRing.GeneralAvailability:
                    return WindowsUpdatePolicyDataContract.JsonGeneralAvailability;
            }
            throw new Error(ErrorCodes.E_NOTIMPL, "WindowsUpdateRing value is not implemented (" + ring.ToString() + ").");
        }

        public static WindowsUpdateRing RingFromJsonString(string ring)
        {
            if (ring == WindowsUpdatePolicyDataContract.JsonEarlyAdopter)
            {
                return WindowsUpdateRing.EarlyAdopter;
            }
            else if (ring == WindowsUpdatePolicyDataContract.JsonPreview)
            {
                return WindowsUpdateRing.Preview;
            }
            else if (ring == WindowsUpdatePolicyDataContract.JsonGeneralAvailability)
            {
                return WindowsUpdateRing.GeneralAvailability;
            }
            throw new Error(ErrorCodes.E_NOTIMPL, "WindowsUpdateRing value is not implemented (" + ring + ").");
        }
    }

    class WindowsUpdatePolicyHandler : IClientPropertyHandler
    {
        const string JsonSectionName = "windowsUpdatePolicy";

        public WindowsUpdatePolicyHandler(IClientHandlerCallBack callback, ISystemConfiguratorProxy systemConfiguratorProxy)
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
            if (!(desiredValue is JObject))
            {
                return;
            }

            WindowsUpdatePolicyDataContract.DesiredProperties desiredProperties = WindowsUpdatePolicyDataContract.DesiredProperties.FromJsonObject((JObject)desiredValue);

            if (desiredProperties.applyProperties != null)
            {
                Message.SetWindowsUpdatePolicyRequest request = new Message.SetWindowsUpdatePolicyRequest();
                request.ApplyFromDeviceTwin = desiredProperties.applyProperties != null ? WindowsUpdatePolicyDataContract.JsonYes : WindowsUpdatePolicyDataContract.JsonNo;
                request.ReportToDeviceTwin = desiredProperties.reportProperties;
                request.data = new Message.WindowsUpdatePolicyConfiguration();
                request.data.activeFields = 0;

                // Construct the request and send it...
                Message.Policy policy = new Message.Policy();
                policy.source = Message.PolicySource.Remote;
                policy.sourcePriorities = desiredProperties.applyProperties.sourcePriority == PolicyDataContract.JsonLocal ? _priorityLocal : _priorityRemote;
                request.data.policy = policy;

                if (desiredProperties.applyProperties.activeHoursStart != -1)
                {
                    request.data.activeHoursStart = (uint)desiredProperties.applyProperties.activeHoursStart;
                    request.data.activeFields |= (uint)Message.ActiveFields.ActiveHoursStart;
                }

                if (desiredProperties.applyProperties.activeHoursEnd != -1)
                {
                    request.data.activeHoursEnd = (uint)desiredProperties.applyProperties.activeHoursEnd;
                    request.data.activeFields |= (uint)Message.ActiveFields.ActiveHoursEnd;
                }

                if (desiredProperties.applyProperties.allowAutoUpdate != -1)
                {
                    request.data.allowAutoUpdate = (uint)desiredProperties.applyProperties.allowAutoUpdate;
                    request.data.activeFields |= (uint)Message.ActiveFields.AllowAutoUpdate;
                }

                if (desiredProperties.applyProperties.allowUpdateService != -1)
                {
                    request.data.allowUpdateService = (uint)desiredProperties.applyProperties.allowUpdateService;
                    request.data.activeFields |= (uint)Message.ActiveFields.AllowUpdateService;
                }

                if (desiredProperties.applyProperties.branchReadinessLevel != -1)
                {
                    request.data.branchReadinessLevel = (uint)desiredProperties.applyProperties.branchReadinessLevel;
                    request.data.activeFields |= (uint)Message.ActiveFields.BranchReadinessLevel;
                }

                if (desiredProperties.applyProperties.deferFeatureUpdatesPeriod != -1)
                {
                    request.data.deferFeatureUpdatesPeriod = (uint)desiredProperties.applyProperties.deferFeatureUpdatesPeriod;
                    request.data.activeFields |= (uint)Message.ActiveFields.DeferFeatureUpdatesPeriod;
                }

                if (desiredProperties.applyProperties.deferQualityUpdatesPeriod != -1)
                {
                    request.data.deferQualityUpdatesPeriod = (uint)desiredProperties.applyProperties.deferQualityUpdatesPeriod;
                    request.data.activeFields |= (uint)Message.ActiveFields.DeferQualityUpdatesPeriod;
                }

                if (desiredProperties.applyProperties.pauseFeatureUpdates != -1)
                {
                    request.data.pauseFeatureUpdates = (uint)desiredProperties.applyProperties.pauseFeatureUpdates;
                    request.data.activeFields |= (uint)Message.ActiveFields.PauseFeatureUpdates;
                }

                if (desiredProperties.applyProperties.pauseQualityUpdates != -1)
                {
                    request.data.pauseQualityUpdates = (uint)desiredProperties.applyProperties.pauseQualityUpdates;
                    request.data.activeFields |= (uint)Message.ActiveFields.PauseQualityUpdates;
                }

                if (desiredProperties.applyProperties.scheduledInstallDay != -1)
                {
                    request.data.scheduledInstallDay = (uint)desiredProperties.applyProperties.scheduledInstallDay;
                    request.data.activeFields |= (uint)Message.ActiveFields.ScheduledInstallDay;
                }

                if (desiredProperties.applyProperties.scheduledInstallTime != -1)
                {
                    request.data.scheduledInstallTime = (uint)desiredProperties.applyProperties.scheduledInstallTime;
                    request.data.activeFields |= (uint)Message.ActiveFields.ScheduledInstallTime;
                }

                if (desiredProperties.applyProperties.ring != WindowsUpdatePolicyDataContract.NotFound)
                {
                    request.data.ring = desiredProperties.applyProperties.ring;
                    request.data.activeFields |= (uint)Message.ActiveFields.Ring;
                }

                // Always send down to SystemConfigurator because we need to persist the reporting (if specified).
                await this._systemConfiguratorProxy.SendCommandAsync(request);
            }

            await ReportToDeviceTwin();
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
            Message.GetWindowsUpdatePolicyResponse reportedProperties = await GetWindowsUpdatePolicyAsync();
            return JObject.FromObject(reportedProperties);
        }

        private async Task ReportToDeviceTwin()
        {
            Message.GetWindowsUpdatePolicyResponse response = await GetWindowsUpdatePolicyAsync();

            if (response.ReportToDeviceTwin == DMJSonConstants.YesString)
            {
                WindowsUpdatePolicyDataContract.WUProperties reportedProperties = new WindowsUpdatePolicyDataContract.WUProperties();

                reportedProperties.activeHoursStart = (int)response.data.activeHoursStart;
                reportedProperties.activeHoursEnd = (int)response.data.activeHoursEnd;
                reportedProperties.allowAutoUpdate = (int)response.data.allowAutoUpdate;
                reportedProperties.allowUpdateService = (int)response.data.allowUpdateService;
                reportedProperties.branchReadinessLevel = (int)response.data.branchReadinessLevel;

                reportedProperties.deferFeatureUpdatesPeriod = (int)response.data.deferFeatureUpdatesPeriod;
                reportedProperties.deferQualityUpdatesPeriod = (int)response.data.deferQualityUpdatesPeriod;
                reportedProperties.pauseFeatureUpdates = (int)response.data.pauseFeatureUpdates;
                reportedProperties.pauseQualityUpdates = (int)response.data.pauseQualityUpdates;
                reportedProperties.scheduledInstallDay = (int)response.data.scheduledInstallDay;

                reportedProperties.scheduledInstallTime = (int)response.data.scheduledInstallTime;

                reportedProperties.ring = response.data.ring;

                reportedProperties.sourcePriority = PolicyHelpers.SourcePriorityFromPolicy(response.data.policy);

                await this._callback.ReportPropertiesAsync(JsonSectionName, reportedProperties.ToJsonObject());
            }
            else
            {
                await this._callback.ReportPropertiesAsync(JsonSectionName, DMJSonConstants.NoReportString);
            }
        }

        public async Task<Message.GetWindowsUpdatePolicyResponse> GetWindowsUpdatePolicyAsync()
        {
            var request = new Message.GetWindowsUpdatePolicyRequest();
            var response = await this._systemConfiguratorProxy.SendCommandAsync(request);
            return response as Message.GetWindowsUpdatePolicyResponse;
        }

        public async Task SetRingAsync(WindowsUpdateRingState userDesiredState)
        {
            // Construct the request and send it...
            Message.Policy policy = new Message.Policy();
            policy.source = Message.PolicySource.Local;
            policy.sourcePriorities = userDesiredState.settingsPriority == SettingsPriority.Local ? PolicyHelpers.PriorityLocal : PolicyHelpers.PriorityRemote;

            Message.SetWindowsUpdatePolicyRequest request = new Message.SetWindowsUpdatePolicyRequest();
            request.ApplyFromDeviceTwin = WindowsUpdatePolicyDataContract.JsonYes;
            request.data = new Message.WindowsUpdatePolicyConfiguration();
            request.data.activeFields = (uint)Message.ActiveFields.Ring;
            request.data.ring = WindowsUpdateRingState.RingToJsonString(userDesiredState.ring);
            request.data.policy = policy;
            request.ReportToDeviceTwin = Constants.JsonValueUnspecified;    // Keep whatever already stored.
            await this._systemConfiguratorProxy.SendCommandAsync(request);

            await ReportToDeviceTwin();
        }

        public static SettingsPriority SettingsPriorityFromString(Message.PolicySource s)
        {
            switch (s)
            {
                case Message.PolicySource.Local:
                    return SettingsPriority.Local;
                case Message.PolicySource.Remote:
                    return SettingsPriority.Remote;
            }
            return SettingsPriority.Unknown;
        }

        public async Task<WindowsUpdateRingState> GetRingAsync()
        {
            Message.GetWindowsUpdatePolicyResponse response = await GetWindowsUpdatePolicyAsync();

            WindowsUpdateRingState state = new WindowsUpdateRingState();
            state.ring = WindowsUpdateRingState.RingFromJsonString(response.data.ring);
            state.settingsPriority = SettingsPriorityFromString(response.data.policy.source);
            return state;
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
