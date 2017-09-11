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
    }

    class WindowsUpdatePolicyHandler : IClientPropertyHandler
    {
        const string JsonSectionName = "windowsUpdatePolicy";

        private static string RingToJsonString(WindowsUpdateRing ring)
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

        private static WindowsUpdateRing RingFromJsonString(string ring)
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

            WindowsUpdatePolicyDataContract.DesiredProperties desiredProperties = new WindowsUpdatePolicyDataContract.DesiredProperties();
            desiredProperties.LoadFrom((JObject)desiredValue);

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

                if (desiredProperties.applyProperties.activeHoursStart != WindowsUpdatePolicyDataContract.NotFound)
                {
                    request.data.activeHoursStart = UInt32.Parse(desiredProperties.applyProperties.activeHoursStart);
                    request.data.activeFields |= (uint)Message.ActiveFields.ActiveHoursStart;
                }

                if (desiredProperties.applyProperties.activeHoursEnd != WindowsUpdatePolicyDataContract.NotFound)
                {
                    request.data.activeHoursEnd = UInt32.Parse(desiredProperties.applyProperties.activeHoursEnd);
                    request.data.activeFields |= (uint)Message.ActiveFields.ActiveHoursEnd;
                }

                if (desiredProperties.applyProperties.allowAutoUpdate != WindowsUpdatePolicyDataContract.NotFound)
                {
                    request.data.allowAutoUpdate = UInt32.Parse(desiredProperties.applyProperties.allowAutoUpdate);
                    request.data.activeFields |= (uint)Message.ActiveFields.AllowAutoUpdate;
                }

                if (desiredProperties.applyProperties.allowUpdateService != WindowsUpdatePolicyDataContract.NotFound)
                {
                    request.data.allowUpdateService = UInt32.Parse(desiredProperties.applyProperties.allowUpdateService);
                    request.data.activeFields |= (uint)Message.ActiveFields.AllowUpdateService;
                }

                if (desiredProperties.applyProperties.branchReadinessLevel != WindowsUpdatePolicyDataContract.NotFound)
                {
                    request.data.branchReadinessLevel = UInt32.Parse(desiredProperties.applyProperties.branchReadinessLevel);
                    request.data.activeFields |= (uint)Message.ActiveFields.BranchReadinessLevel;
                }

                if (desiredProperties.applyProperties.deferFeatureUpdatesPeriod != WindowsUpdatePolicyDataContract.NotFound)
                {
                    request.data.deferFeatureUpdatesPeriod = UInt32.Parse(desiredProperties.applyProperties.deferFeatureUpdatesPeriod);
                    request.data.activeFields |= (uint)Message.ActiveFields.DeferFeatureUpdatesPeriod;
                }

                if (desiredProperties.applyProperties.deferQualityUpdatesPeriod != WindowsUpdatePolicyDataContract.NotFound)
                {
                    request.data.deferQualityUpdatesPeriod = UInt32.Parse(desiredProperties.applyProperties.deferQualityUpdatesPeriod);
                    request.data.activeFields |= (uint)Message.ActiveFields.DeferQualityUpdatesPeriod;
                }

                if (desiredProperties.applyProperties.pauseFeatureUpdates != WindowsUpdatePolicyDataContract.NotFound)
                {
                    request.data.pauseFeatureUpdates = UInt32.Parse(desiredProperties.applyProperties.pauseFeatureUpdates);
                    request.data.activeFields |= (uint)Message.ActiveFields.PauseFeatureUpdates;
                }

                if (desiredProperties.applyProperties.pauseQualityUpdates != WindowsUpdatePolicyDataContract.NotFound)
                {
                    request.data.pauseQualityUpdates = UInt32.Parse(desiredProperties.applyProperties.pauseQualityUpdates);
                    request.data.activeFields |= (uint)Message.ActiveFields.PauseQualityUpdates;
                }

                if (desiredProperties.applyProperties.scheduledInstallDay != WindowsUpdatePolicyDataContract.NotFound)
                {
                    request.data.scheduledInstallDay = UInt32.Parse(desiredProperties.applyProperties.scheduledInstallDay);
                    request.data.activeFields |= (uint)Message.ActiveFields.ScheduledInstallDay;
                }

                if (desiredProperties.applyProperties.scheduledInstallTime != WindowsUpdatePolicyDataContract.NotFound)
                {
                    request.data.scheduledInstallTime = UInt32.Parse(desiredProperties.applyProperties.scheduledInstallTime);
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

                reportedProperties.activeHoursStart = response.data.activeHoursStart.ToString();
                reportedProperties.activeHoursEnd = response.data.activeHoursEnd.ToString();
                reportedProperties.allowAutoUpdate = response.data.allowAutoUpdate.ToString();
                reportedProperties.allowUpdateService = response.data.allowUpdateService.ToString();
                reportedProperties.branchReadinessLevel = response.data.branchReadinessLevel.ToString();

                reportedProperties.deferFeatureUpdatesPeriod = response.data.deferFeatureUpdatesPeriod.ToString();
                reportedProperties.deferQualityUpdatesPeriod = response.data.deferQualityUpdatesPeriod.ToString();
                reportedProperties.pauseFeatureUpdates = response.data.pauseFeatureUpdates.ToString();
                reportedProperties.pauseQualityUpdates = response.data.pauseQualityUpdates.ToString();
                reportedProperties.scheduledInstallDay = response.data.scheduledInstallDay.ToString();

                reportedProperties.scheduledInstallTime = response.data.scheduledInstallTime.ToString();

                reportedProperties.ring = response.data.ring;

                reportedProperties.sourcePriority = PolicyHelpers.SourcePriorityFromPolicy(response.data.policy);

                await this._callback.ReportPropertiesAsync(JsonSectionName, reportedProperties.ToJson());
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
            request.data.ring = RingToJsonString(userDesiredState.ring);
            request.data.policy = policy;
            request.ReportToDeviceTwin = Constants.JsonValueUnspecified;    // Keep whatever already stored.
            await this._systemConfiguratorProxy.SendCommandAsync(request);

            await ReportToDeviceTwin();
        }

        public async Task<WindowsUpdateRingState> GetRingAsync()
        {
            Message.GetWindowsUpdatePolicyResponse response = await GetWindowsUpdatePolicyAsync();

            WindowsUpdateRingState state = new WindowsUpdateRingState();
            state.ring = RingFromJsonString(response.data.ring);
            state.settingsPriority = PolicyHelpers.SettingsPriorityFromString(response.data.policy.source);
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
