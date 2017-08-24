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
using Microsoft.Devices.Management.Message;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using Windows.Foundation.Diagnostics;

namespace Microsoft.Devices.Management
{
    class RebootCmdHandler : IClientDirectMethodHandler
    {
        public string PropertySectionName
        {
            get
            {
                return RebootCmdDataContract.SectionName;
            }
        }

        public RebootCmdHandler(IClientHandlerCallBack callback,
            ISystemConfiguratorProxy systemConfiguratorProxy,
            IDeviceManagementRequestHandler hostAppHandler,
            WindowsUpdatePolicyHandler windowsUpdatePolicyHandler)
        {
            _systemConfiguratorProxy = systemConfiguratorProxy;
            _deviceManagementClient = callback;
            _hostAppHandler = hostAppHandler;
            _windowsUpdatePolicyHandler = windowsUpdatePolicyHandler;
        }

        public IReadOnlyDictionary<string, Func<string, Task<string>>> GetDirectMethodHandler()
        {
            return new Dictionary<string, Func<string, Task<string>>>()
                {
                   { RebootCmdDataContract.RebootCmdAsync, RebootCmdAsyncHandler }
                };
        }

        private async Task ReportRebootCmdStatus(RebootCmdDataContract.ResponseValue status, string rebootCmdTime)
        {
            Logger.Log("ReportRebootCmdStatus() invoked.", LoggingLevel.Verbose);

            RebootCmdDataContract.ReportedProperties reportedProperties = new RebootCmdDataContract.ReportedProperties(status);
            await _deviceManagementClient.ReportPropertiesAsync(PropertySectionName, JObject.FromObject(reportedProperties));

            StatusSection statusSection = new StatusSection(StatusSection.StateType.Completed);
            await _deviceManagementClient.ReportStatusAsync(PropertySectionName, statusSection);
        }

        private async Task<RebootCmdDataContract.ResponseValue> IsRebootAllowedByApp()
        {
            Logger.Log("IsRebootAllowedByApp() invoked.", LoggingLevel.Verbose);

            bool allowed = await _hostAppHandler.IsSystemRebootAllowed();
            if (!allowed)
            {
                return RebootCmdDataContract.ResponseValue.RejectedByApp;
            }

            return RebootCmdDataContract.ResponseValue.Allowed;
        }

        private async Task InternalRebootCmdAsync(string rebootCmdTime)
        {
            Logger.Log("InternalRebootCmdAsync(" + rebootCmdTime + ") invoked.", LoggingLevel.Verbose);

            StatusSection status = new StatusSection(StatusSection.StateType.Pending);
            await _deviceManagementClient.ReportStatusAsync(PropertySectionName, status);

            var response = await IsRebootAllowedBySystem();
            if (response != RebootCmdDataContract.ResponseValue.Allowed)
            {
                await ReportRebootCmdStatus(response, rebootCmdTime);
                return;
            }

            response = await IsRebootAllowedByApp();
            if (response != RebootCmdDataContract.ResponseValue.Allowed)
            {
                await ReportRebootCmdStatus(response, rebootCmdTime);
                return;
            }

            {
                var request = new ImmediateRebootRequest();
                request.lastRebootCmdTime = rebootCmdTime;
                // await this._systemConfiguratorProxy.SendCommandAsync(request);

                response = RebootCmdDataContract.ResponseValue.Scheduled;
                await ReportRebootCmdStatus(response, rebootCmdTime);
                return;
            }
        }

        private async Task InternalRebootCmdAsync()
        {
            Logger.Log("InternalRebootCmdAsync() invoked.", LoggingLevel.Verbose);

            await Helpers.EnsureErrorsLogged(_deviceManagementClient, PropertySectionName, async () =>
            {
                string rebootCmdTime = DateTime.Now.ToString("yyyy-MM-ddTHH:mm:ssZ");
                await InternalRebootCmdAsync(rebootCmdTime);
            });
        }

        private Task<string> RebootCmdAsyncHandler(string jsonParam)
        {
            Logger.Log("RebootCmdAsyncHandler() invoked by direct method.", LoggingLevel.Verbose);

            InternalRebootCmdAsync().FireAndForget();  // Will do its own error reporting.

            StatusSection status = new StatusSection(StatusSection.StateType.Pending);
            return Task.FromResult<string>(status.AsJsonObject().ToString());
        }

        // For public APIs, we do not report error. We let the caller handle/report the error.
        public async Task RebootAsync()
        {
            Logger.Log("RebootAsync()", LoggingLevel.Verbose);

            string rebootCmdTime = DateTime.Now.ToString("yyyy-MM-ddTHH:mm:ssZ");

            await InternalRebootCmdAsync(rebootCmdTime);
        }

        // For public APIs, we do not report error. We let the caller handle/report the error.
        public async Task<RebootCmdDataContract.ResponseValue> IsRebootAllowedBySystem()
        {
            Logger.Log("IsRebootAllowedBySystem()", LoggingLevel.Verbose);

            var request = new GetWindowsUpdateRebootPolicyRequest();
            var response = await this._systemConfiguratorProxy.SendCommandAsync(request) as GetWindowsUpdateRebootPolicyResponse;
            if (!response.configuration.allow)
            {
                return RebootCmdDataContract.ResponseValue.Disabled;
            }

            GetWindowsUpdatePolicyResponse updatePolicy = await _windowsUpdatePolicyHandler.GetWindowsUpdatePolicyAsync();
            if (updatePolicy.data != null)
            {
                uint nowHour = (uint)DateTime.Now.Hour;
                if (updatePolicy.data.activeHoursStart <= nowHour && nowHour < updatePolicy.data.activeHoursEnd)
                {
                    return RebootCmdDataContract.ResponseValue.InActiveHours;
                }
            }

            return RebootCmdDataContract.ResponseValue.Allowed;
        }

        // For public APIs, we do not report error. We let the caller handle/report the error.
        public async Task AllowReboots(bool allowReboots)
        {
            var configuration = new WindowsUpdateRebootPolicyConfiguration();
            configuration.allow = allowReboots;
            await this._systemConfiguratorProxy.SendCommandAsync(new SetWindowsUpdateRebootPolicyRequest(configuration));
        }

        private ISystemConfiguratorProxy _systemConfiguratorProxy;
        private IClientHandlerCallBack _deviceManagementClient;
        private IDeviceManagementRequestHandler _hostAppHandler;
        private WindowsUpdatePolicyHandler _windowsUpdatePolicyHandler;
    }
}