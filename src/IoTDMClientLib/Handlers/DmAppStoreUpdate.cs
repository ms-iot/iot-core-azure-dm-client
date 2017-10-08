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
using System.Collections.Generic;
using System.Threading.Tasks;
using Windows.Foundation;
using Windows.Foundation.Diagnostics;
using Windows.Services.Store;

namespace Microsoft.Devices.Management
{
    class DmAppStoreUpdateHandler : IClientDirectMethodHandler
    {
        public string PropertySectionName
        {
            get
            {
                return DmAppStoreUpdateDataContract.SectionName;
            }
        }

        public DmAppStoreUpdateHandler(IClientHandlerCallBack callback, ISystemConfiguratorProxy systemConfiguratorProxy)
        {
            _systemConfiguratorProxy = systemConfiguratorProxy;
            _deviceManagementClient = callback;
        }

        public IReadOnlyDictionary<string, Func<string, Task<string>>> GetDirectMethodHandler()
        {
            return new Dictionary<string, Func<string, Task<string>>>()
                {
                    { DmAppStoreUpdateDataContract.StartDmAppStoreUpdateAsync, StartDmAppStoreUpdateAsync }
                };
        }

        private async Task ReportResponse(string response, string lastCheck)
        {
            Logger.Log("ReportResponse() invoked. Response = " + response, LoggingLevel.Verbose);

            DmAppStoreUpdateDataContract.ReportedProperties reportedProperties = new DmAppStoreUpdateDataContract.ReportedProperties();
            reportedProperties.response = response;
            reportedProperties.lastCheck = lastCheck;
            await _deviceManagementClient.ReportPropertiesAsync(PropertySectionName, reportedProperties.ToJsonObject());
        }

        private async Task InternalStartDmAppStoreUpdateAsync(string jsonParamString)
        {
            Logger.Log("InternalStartDmAppStoreUpdateAsync() invoked.", LoggingLevel.Verbose);

            await Helpers.EnsureErrorsLogged(_deviceManagementClient, PropertySectionName, async () =>
            {
                // Report to the device twin
                StatusSection status = new StatusSection(StatusSection.StateType.Pending);
                await _deviceManagementClient.ReportStatusAsync(PropertySectionName, status);

                StoreContext context = StoreContext.GetDefault();

                // Check for updates...
                string lastCheck = DateTime.Now.ToString("yyyy-MM-ddTHH:mm:ssZ");

                await ReportResponse(DmAppStoreUpdateDataContract.JSonChecking, lastCheck);

                IReadOnlyList<StorePackageUpdate> updates = await context.GetAppAndOptionalStorePackageUpdatesAsync();
                if (updates.Count == 0)
                {
                    await ReportResponse(DmAppStoreUpdateDataContract.JSonNoUpdates, lastCheck);
                    return;
                }

                // Download and install the updates...
                IAsyncOperationWithProgress<StorePackageUpdateResult, StorePackageUpdateStatus> downloadOperation =
                    context.RequestDownloadAndInstallStorePackageUpdatesAsync(updates);

                await ReportResponse(DmAppStoreUpdateDataContract.JsonDownloadingAndInstalling, lastCheck);

                // Wait for completion...
                StorePackageUpdateResult result = await downloadOperation.AsTask();

                string resultString = result.OverallState == StorePackageUpdateState.Completed ?
                    DmAppStoreUpdateDataContract.JsonInstalled :
                    DmAppStoreUpdateDataContract.JsonFailed;

                await ReportResponse(resultString, lastCheck);

                // Report to the device twin
                status.State = StatusSection.StateType.Completed;
                await _deviceManagementClient.ReportStatusAsync(PropertySectionName, status);
            });
        }

        private Task<string> StartDmAppStoreUpdateAsync(string jsonParam)
        {
            Logger.Log("StartDmAppStoreUpdateAsync() invoked by direct method.", LoggingLevel.Verbose);

            InternalStartDmAppStoreUpdateAsync(jsonParam).FireAndForget();  // Will do its own error reporting.

            StatusSection status = new StatusSection(StatusSection.StateType.Pending);
            return Task.FromResult<string>(status.AsJsonObject().ToString());
        }

        private ISystemConfiguratorProxy _systemConfiguratorProxy;
        private IClientHandlerCallBack _deviceManagementClient;
    }
}
