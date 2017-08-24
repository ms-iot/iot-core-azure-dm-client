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
using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using Windows.Foundation.Diagnostics;

namespace Microsoft.Devices.Management
{
    class AppxLifeCycleHandler : IClientDirectMethodHandler
    {
        public string PropertySectionName
        {
            get
            {
                return AppxLifeCycleDataContract.SectionName;
            }
        }

        public AppxLifeCycleHandler(IClientHandlerCallBack callback, ISystemConfiguratorProxy systemConfiguratorProxy)
        {
            _systemConfiguratorProxy = systemConfiguratorProxy;
            _deviceManagementClient = callback;
        }

        public IReadOnlyDictionary<string, Func<string, Task<string>>> GetDirectMethodHandler()
        {
            return new Dictionary<string, Func<string, Task<string>>>()
                {
                   { AppxLifeCycleDataContract.ManageAppLifeCycleAsync, ManageAppLifeCycleAsyncHandler }
                };
        }

        private async Task InternalManageAppLifeCycleAsync(string jsonParam)
        {
            Logger.Log("InternalManageAppLifeCycleAsync() invoked.", LoggingLevel.Verbose);

            await Helpers.EnsureErrorsLogged(_deviceManagementClient, PropertySectionName, async () =>
            {
                StatusSection status = new StatusSection(StatusSection.StateType.Pending);
                await _deviceManagementClient.ReportStatusAsync(PropertySectionName, status);

                // Parse json parameters
                AppxLifeCycleDataContract.ManageAppLifeCycleParams parameters = new AppxLifeCycleDataContract.ManageAppLifeCycleParams();
                parameters.LoadFrom(jsonParam);

                // Construct request
                Message.AppLifecycleInfo appLifeCycleInfo = new Message.AppLifecycleInfo();
                appLifeCycleInfo.AppId = parameters.pkgFamilyName;
                appLifeCycleInfo.Start = parameters.action == AppxLifeCycleDataContract.JsonStart;
                var request = new Message.AppLifecycleRequest(appLifeCycleInfo);

                // Send the request
                await _systemConfiguratorProxy.SendCommandAsync(request);

                // Report to the device twin
                status.State = StatusSection.StateType.Completed;
                await _deviceManagementClient.ReportStatusAsync(PropertySectionName, status);
            });
        }

        private Task<string> ManageAppLifeCycleAsyncHandler(string jsonParam)
        {
            Logger.Log("ManageAppLifeCycleAsyncHandler() invoked by direct method.", LoggingLevel.Verbose);

            InternalManageAppLifeCycleAsync(jsonParam).FireAndForget();  // Will do its own error reporting.

            StatusSection status = new StatusSection(StatusSection.StateType.Pending);
            return Task.FromResult<string>(status.AsJsonObject().ToString());
        }

        private ISystemConfiguratorProxy _systemConfiguratorProxy;
        private IClientHandlerCallBack _deviceManagementClient;
    }
}