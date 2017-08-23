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

using Microsoft.Azure.Devices.Shared;
using Microsoft.Devices.Management.DMDataContract;
using Microsoft.Devices.Management.Message;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using Windows.Foundation.Diagnostics;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using Windows.Foundation;
using Windows.Services.Store;
using Windows.Storage;

namespace Microsoft.Devices.Management
{
    // This is the main entry point into DM
    public class DeviceManagementClient : IClientHandlerCallBack
    {
        const string MethodStartAppSelfUpdate = DMJSonConstants.DTWindowsIoTNameSpace + ".startAppSelfUpdate";
        const string MethodGetCertificateDetails = DMJSonConstants.DTWindowsIoTNameSpace + ".getCertificateDetails";
        const string MethodManageAppLifeCycle = DMJSonConstants.DTWindowsIoTNameSpace + ".manageAppLifeCycle";

        public IDeviceTwin DeviceTwin { get { return _deviceTwin; } }

        public struct GetCertificateDetailsParams
        {
            public string path;
            public string hash;
            public string connectionString;
            public string containerName;
            public string blobName;
        }

        class AppLifeCycleParameters
        {
            public string pkgFamilyName = "";
            public string action = "";
        }

        class StartupApps
        {
            public string foreground = "";
        }

        private DeviceManagementClient(IDeviceTwin deviceTwin, IDeviceManagementRequestHandler hostAppHandler, ISystemConfiguratorProxy systemConfiguratorProxy)
        {
            Logger.Log("Entering DeviceManagementClient constructor.", LoggingLevel.Verbose);

            this._deviceTwin = deviceTwin;
            this._hostAppHandler = hostAppHandler;
            this._systemConfiguratorProxy = systemConfiguratorProxy;
            this._desiredPropertyMap = new Dictionary<string, IClientPropertyHandler>();
            this._desiredPropertyDependencyMap = new Dictionary<string, List<IClientPropertyDependencyHandler>>();
        }

        private void AddPropertyHandler(IClientPropertyHandler handler)
        {
            Logger.Log("Adding property handler.", LoggingLevel.Verbose);

            this._desiredPropertyMap.Add(handler.PropertySectionName, handler);

            var handlerWithDependencies = handler as IClientPropertyDependencyHandler;
            if (handlerWithDependencies != null)
            {
                foreach (var dependencySection in handlerWithDependencies.PropertySectionDependencyNames)
                {
                    AddPropertyDependencyHandler(dependencySection, handlerWithDependencies);
                }
            }
        }

        private void AddPropertyDependencyHandler(string sectionName, IClientPropertyDependencyHandler handler)
        {
            Logger.Log("Adding property dependency handler.", LoggingLevel.Verbose);

            List<IClientPropertyDependencyHandler> handlerList;
            if (!this._desiredPropertyDependencyMap.TryGetValue(sectionName, out handlerList))
            {
                handlerList = new List<IClientPropertyDependencyHandler>();
                this._desiredPropertyDependencyMap.Add(sectionName, handlerList);
            }
            handlerList.Add(handler);
        }

        private async Task AddDirectMethodHandlerAsync(IClientDirectMethodHandler handler)
        {
            Logger.Log("Adding direct method handler async.", LoggingLevel.Verbose);

            foreach (var pair in handler.GetDirectMethodHandler())
            {
                var guard = new DirectMethodGuard(pair.Key, pair.Value);
                await this._deviceTwin.SetMethodHandlerAsync(pair.Key, guard.Invoke);
            }
        }

        public static async Task<DeviceManagementClient> CreateAsync(IDeviceTwin deviceTwin, IDeviceManagementRequestHandler hostAppHandler)
        {
            Logger.Log("Creating Device Management objects.", LoggingLevel.Verbose);

            var systemConfiguratorProxy = new SystemConfiguratorProxy();
            DeviceManagementClient deviceManagementClient = Create(deviceTwin, hostAppHandler, systemConfiguratorProxy);
            IClientHandlerCallBack clientCallback = deviceManagementClient;

            // Attach methods...
            await deviceTwin.SetMethodHandlerAsync(CommonDataContract.ReportAllAsync, deviceManagementClient.ReportAllDevicePropertiesMethodHandler);
            await deviceTwin.SetMethodHandlerAsync(MethodStartAppSelfUpdate, deviceManagementClient.StartAppSelfUpdateMethodHandlerAsync);
            await deviceTwin.SetMethodHandlerAsync(MethodGetCertificateDetails, deviceManagementClient.GetCertificateDetailsHandlerAsync);
            await deviceTwin.SetMethodHandlerAsync(MethodManageAppLifeCycle, deviceManagementClient.ManageAppLifeCycleHandlerAsync);

            // Create/Attach handlers...
            deviceManagementClient._externalStorageHandler = new ExternalStorageHandler();
            deviceManagementClient.AddPropertyHandler(deviceManagementClient._externalStorageHandler);

            var deviceHealthAttestationHandler = new DeviceHealthAttestationHandler(clientCallback, systemConfiguratorProxy);
            deviceManagementClient.AddPropertyHandler(deviceHealthAttestationHandler);
            await deviceManagementClient.AddDirectMethodHandlerAsync(deviceHealthAttestationHandler);

            deviceManagementClient._factoryResetHandler = new FactoryResetHandler(clientCallback, systemConfiguratorProxy);
            await deviceManagementClient.AddDirectMethodHandlerAsync(deviceManagementClient._factoryResetHandler);

            deviceManagementClient._windowsUpdatePolicyHandler = new WindowsUpdatePolicyHandler(clientCallback, systemConfiguratorProxy);
            deviceManagementClient.AddPropertyHandler(deviceManagementClient._windowsUpdatePolicyHandler);

            var wifiHandler = new WifiHandler(clientCallback, systemConfiguratorProxy);
            deviceManagementClient.AddPropertyHandler(wifiHandler);
            await deviceManagementClient.AddDirectMethodHandlerAsync(wifiHandler);

            var appxHandler = new AppxManagement(clientCallback, systemConfiguratorProxy, deviceManagementClient._desiredCache);
            deviceManagementClient.AddPropertyHandler(appxHandler);

            var eventTracingHandler = new EventTracingHandler(clientCallback, systemConfiguratorProxy, deviceManagementClient._desiredCache);
            deviceManagementClient.AddPropertyHandler(eventTracingHandler);

            var storageHandler = new StorageHandler(clientCallback, systemConfiguratorProxy);
            await deviceManagementClient.AddDirectMethodHandlerAsync(storageHandler);

            var timeSettingsHandler = new TimeSettingsHandler(clientCallback, systemConfiguratorProxy);
            deviceManagementClient.AddPropertyHandler(timeSettingsHandler);

            var timeServiceHandler = new TimeServiceHandler(clientCallback, systemConfiguratorProxy);
            deviceManagementClient.AddPropertyHandler(timeServiceHandler);

            deviceManagementClient._rebootCmdHandler = new RebootCmdHandler(
                clientCallback,
                systemConfiguratorProxy,
                deviceManagementClient._hostAppHandler,
                deviceManagementClient._windowsUpdatePolicyHandler);
            await deviceManagementClient.AddDirectMethodHandlerAsync(deviceManagementClient._rebootCmdHandler);

            var rebootInfoHandler = new RebootInfoHandler(
                clientCallback,
                systemConfiguratorProxy,
                deviceManagementClient._desiredCache);
            deviceManagementClient.AddPropertyHandler(rebootInfoHandler);

            deviceManagementClient._windowsTelemetryHandler = new WindowsTelemetryHandler(
                clientCallback,
                systemConfiguratorProxy);
            deviceManagementClient.AddPropertyHandler(deviceManagementClient._windowsTelemetryHandler);

            var deviceInfoHandler = new DeviceInfoHandler(systemConfiguratorProxy);
            deviceManagementClient.AddPropertyHandler(deviceInfoHandler);

            return deviceManagementClient;
        }

        internal static DeviceManagementClient Create(IDeviceTwin deviceTwin, IDeviceManagementRequestHandler requestHandler, ISystemConfiguratorProxy systemConfiguratorProxy)
        {
            return new DeviceManagementClient(deviceTwin, requestHandler, systemConfiguratorProxy);
        }

        // IClientHandlerCallBack.ReportPropertiesAsync
        public async Task ReportPropertiesAsync(string sectionName, JToken sectionValue)
        {
            Debug.WriteLine("ReportPropertiesAsync...");

            JObject windowsNodeValue = new JObject();
            windowsNodeValue.Add(sectionName, sectionValue);

            Dictionary<string, object> collection = new Dictionary<string, object>();
            collection[DMJSonConstants.DTWindowsIoTNameSpace] = windowsNodeValue;

            await _deviceTwin.ReportProperties(collection);
        }

        // IClientHandlerCallBack.SendMessageAsync
        public async Task SendMessageAsync(string message, IDictionary<string, string> properties)
        {
            Logger.Log("HandlerCallback.SendMessageAsync", LoggingLevel.Verbose);

            await _deviceTwin.SendMessageAsync(message, properties);
        }

        // IClientHandlerCallBack.ReportStatusAsync
        public async Task ReportStatusAsync(string sectionName, StatusSection statusSubSection)
        {
            // We always construct an object and set a property inside it.
            // This way, we do not overwrite what's already in there.

            // Set the status to refreshing...
            JObject refreshingValue = new JObject();
            refreshingValue.Add(statusSubSection.AsJsonPropertyRefreshing());
            await ReportPropertiesAsync(sectionName, refreshingValue);

            // Set the status to the actual status...
            JObject actualValue = new JObject();
            actualValue.Add(statusSubSection.AsJsonProperty());
            await ReportPropertiesAsync(sectionName, actualValue);
        }

        public async Task ApplyDesiredStateAsync()
        {
            Logger.Log("Retrieving desired state from device twin...", LoggingLevel.Verbose);

            Dictionary<string, object> desiredProperties = await this._deviceTwin.GetDesiredPropertiesAsync();
            object windowsPropValue = null;
            if (desiredProperties.TryGetValue(DMJSonConstants.DTWindowsIoTNameSpace, out windowsPropValue) && windowsPropValue != null && windowsPropValue is JObject)
            {
                await ApplyDesiredStateAsync((JObject)windowsPropValue);
            }
        }

        public void ApplyDesiredStateAsync(TwinCollection desiredProperties)
        {
            Logger.Log("Applying desired state...", LoggingLevel.Verbose);

            try
            {
                JObject windowsPropValue = (JObject)desiredProperties[DMJSonConstants.DTWindowsIoTNameSpace];
                ApplyDesiredStateAsync(windowsPropValue).FireAndForget();
            }
            catch (Exception)
            {
                Debug.WriteLine("No properties.desired." + DMJSonConstants.DTWindowsIoTNameSpace + " is found.");
            }
        }

        /*
        // ToDo: Not implemented in SystemConfigurator.
        private async Task<bool> CheckForUpdatesAsync()
        {
            var request = new Message.CheckForUpdatesRequest();
            var response = await this._systemConfiguratorProxy.SendCommandAsync(request);
            return (response as Message.CheckForUpdatesResponse).UpdatesAvailable;
        }
        */

        public async Task RebootAsync()
        {
            await _rebootCmdHandler.RebootAsync();
        }

        private void ReportSelfUpdateStatus(string lastCheckValue, string statusValue)
        {
            Dictionary<string, object> collection = new Dictionary<string, object>();
            collection[DMJSonConstants.DTWindowsIoTNameSpace] = new
            {
                appUpdate = new
                {
                    lastCheck = lastCheckValue,
                    status = statusValue,
                }
            };
            _deviceTwin.ReportProperties(collection);
        }

        private async Task StartAppSelfUpdate()
        {
            Debug.WriteLine("Check for updates...");
            StoreContext context = StoreContext.GetDefault();

            // Check for updates...
            string lastCheck = DateTime.Now.ToString("yyyy-MM-ddTHH:mm:ssZ");

            ReportSelfUpdateStatus(lastCheck, "checkStarting");

            IReadOnlyList<StorePackageUpdate> updates = await context.GetAppAndOptionalStorePackageUpdatesAsync();
            if (updates.Count == 0)
            {
                ReportSelfUpdateStatus(lastCheck, "noUpdates");
                return;
            }

            // Download and install the updates...
            IAsyncOperationWithProgress<StorePackageUpdateResult, StorePackageUpdateStatus> downloadOperation =
                context.RequestDownloadAndInstallStorePackageUpdatesAsync(updates);

            ReportSelfUpdateStatus(lastCheck, "updatesDownloadingAndInstalling");

            // Wait for completion...
            StorePackageUpdateResult result = await downloadOperation.AsTask();

            ReportSelfUpdateStatus(lastCheck, result.OverallState == StorePackageUpdateState.Completed ? "installed" : "failed");

            return;
        }

        private Task<string> StartAppSelfUpdateMethodHandlerAsync(string jsonParam)
        {
            Debug.WriteLine("StartAppSelfUpdateMethodHandlerAsync");

            StartAppSelfUpdate().FireAndForget();

            return Task.FromResult(JsonConvert.SerializeObject(new { response = "succeeded" }));
        }

        private async Task GetCertificateDetailsAsync(string jsonParam)
        {
            GetCertificateDetailsParams parameters = JsonConvert.DeserializeObject<GetCertificateDetailsParams>(jsonParam);

            var request = new Message.GetCertificateDetailsRequest();
            request.path = parameters.path;
            request.hash = parameters.hash;

            Message.GetCertificateDetailsResponse response = await _systemConfiguratorProxy.SendCommandAsync(request) as Message.GetCertificateDetailsResponse;

            string jsonString = JsonConvert.SerializeObject(response);
            Debug.WriteLine("response = " + jsonString);

            var info = new Message.AzureFileTransferInfo()
            {
                ConnectionString = parameters.connectionString,
                ContainerName = parameters.containerName,
                BlobName = parameters.blobName,
                Upload = true,
                LocalPath = ""
            };

            var appLocalDataFile = await ApplicationData.Current.TemporaryFolder.CreateFileAsync(parameters.blobName, CreationCollisionOption.ReplaceExisting);
            using (StreamWriter writer = new StreamWriter(await appLocalDataFile.OpenStreamForWriteAsync()))
            {
                await writer.WriteAsync(jsonString);
            }
            await IoTDMClient.AzureBlobFileTransfer.UploadFile(info, appLocalDataFile);

            await appLocalDataFile.DeleteAsync();
        }

        private Task<string> GetCertificateDetailsHandlerAsync(string jsonParam)
        {
            Debug.WriteLine("GetCertificateDetailsHandlerAsync");

            var response = new { response = "succeeded", reason = "" };
            try
            {
                // Submit the work and return immediately.
                GetCertificateDetailsAsync(jsonParam).FireAndForget();
            }
            catch(Exception e)
            {
                response = new { response = "rejected:", reason = e.Message };
            }

            return Task.FromResult(JsonConvert.SerializeObject(response));
        }

        public async Task StartFactoryResetAsync(bool clearTPM, string recoveryPartitionGUID)
        {
            await _factoryResetHandler.StartFactoryResetAsync(clearTPM, recoveryPartitionGUID);
        }

        public async Task SetWindowsTelemetryLevelAsync(WindowsTelemetryLevel level)
        {
            await _windowsTelemetryHandler.SetLevelAsync(level);
        }

        public async Task<WindowsTelemetryLevel> GetWindowsTelemetryLevelAsync()
        {
            return await _windowsTelemetryHandler.GetLevelAsync();
        }

        private Task<string> ManageAppLifeCycleHandlerAsync(string jsonParam)
        {
            Debug.WriteLine("ManageAppLifeCycleHandlerAsync");

            var response = new { response = "succeeded", reason = "" };
            try
            {
                AppLifeCycleParameters appLifeCycleParameters = JsonConvert.DeserializeObject<AppLifeCycleParameters>(jsonParam);

                Message.AppLifecycleInfo appLifeCycleInfo = new Message.AppLifecycleInfo();
                appLifeCycleInfo.AppId = appLifeCycleParameters.pkgFamilyName;
                appLifeCycleInfo.Start = appLifeCycleParameters.action == "start";
                var request = new Message.AppLifecycleRequest(appLifeCycleInfo);
                _systemConfiguratorProxy.SendCommandAsync(request);
            }
            catch (Exception e)
            {
                response = new { response = "rejected:", reason = e.Message };
            }

            return Task.FromResult(JsonConvert.SerializeObject(response));
        }

        private static async Task ProcessDesiredCertificateConfigurationAsync(
            ISystemConfiguratorProxy systemConfiguratorProxy,
            string connectionString,
            string containerName,
            Message.CertificateConfiguration certificateConfiguration)
        {

            await IoTDMClient.CertificateManagement.DownloadCertificates(systemConfiguratorProxy, connectionString, containerName, certificateConfiguration);
            var request = new Message.SetCertificateConfigurationRequest(certificateConfiguration);
            await systemConfiguratorProxy.SendCommandAsync(request);
        }

        public async Task AllowReboots(bool allowReboots)
        {
            await _rebootCmdHandler.AllowReboots(allowReboots);
        }

        public async Task ApplyDesiredStateAsync(JObject windowsPropValue)
        {
            Logger.Log("Applying " + DMJSonConstants.DTWindowsIoTNameSpace + " node desired state...", LoggingLevel.Verbose);

            // ToDo: We should not throw here. All problems need to be logged.
            Message.CertificateConfiguration certificateConfiguration = null;

            foreach (var sectionProp in windowsPropValue.Children().OfType<JProperty>())
            {
                // Handle any dependencies first
                List<IClientPropertyDependencyHandler> handlers;
                if (this._desiredPropertyDependencyMap.TryGetValue(sectionProp.Name, out handlers))
                {
                    handlers.ForEach((handler) =>
                    {
                        try
                        {
                            Debug.WriteLine($"{sectionProp.Name} = {sectionProp.Value.ToString()}");
                            if (sectionProp.Value is JObject)
                            {
                                handler.OnDesiredPropertyDependencyChange(sectionProp.Name, (JObject)sectionProp.Value);
                            }
                        }
                        catch (Exception e)
                        {
                            Debug.WriteLine($"Exception caught while handling desired property - {sectionProp.Name}");
                            Debug.WriteLine(e);
                            throw;
                        }
                    });
                }
            }

            foreach (var sectionProp in windowsPropValue.Children().OfType<JProperty>())
            {
                // If we've been told to stop, we should not process any desired properties...
                if (_lastCommandStatus == CommandStatus.PendingDMAppRestart)
                {
                    break;
                }

                IClientPropertyHandler handler;
                if (this._desiredPropertyMap.TryGetValue(sectionProp.Name, out handler))
                {
                    StatusSection statusSection = new StatusSection(StatusSection.StateType.Pending);
                    await ReportStatusAsync(sectionProp.Name, statusSection);

                    try
                    {
                        Debug.WriteLine($"{sectionProp.Name} = {sectionProp.Value.ToString()}");
                        if (sectionProp.Value is JValue && sectionProp.Value.Type == JTokenType.String && (string)sectionProp.Value == "refreshing")
                        {
                            continue;
                        }

                        _lastCommandStatus = await handler.OnDesiredPropertyChange(sectionProp.Value);

                        statusSection.State = StatusSection.StateType.Completed;
                        await ReportStatusAsync(sectionProp.Name, statusSection);
                    }
                    catch (Error e)
                    {
                        statusSection.State = StatusSection.StateType.Failed;
                        statusSection.TheError = e;

                        Logger.Log(statusSection.ToString(), LoggingLevel.Error);
                        await ReportStatusAsync(sectionProp.Name, statusSection);
                    }
                    catch (Exception e)
                    {
                        statusSection.State = StatusSection.StateType.Failed;
                        statusSection.TheError = new Error(ErrorSubSystem.Unknown, e.HResult, e.Message);

                        Logger.Log(statusSection.ToString(), LoggingLevel.Error);
                        await ReportStatusAsync(sectionProp.Name, statusSection);
                    }
                }
                else
                {
                    if (sectionProp.Value.Type != JTokenType.Object)
                    {
                        continue;
                    }
                    switch (sectionProp.Name)
                    {
                        case "externalStorage":
                            {
                                Debug.WriteLine("externalStorage = " + sectionProp.Value.ToString());

                                JObject subProperties = (JObject)sectionProp.Value;
                                _externalStorageConnectionString = (string)subProperties.Property("connectionString").Value;
                            }
                            break;
                        case "certificates":
                            {
                                // Capture the configuration here.
                                // To apply the configuration we need to wait until externalStorage has been configured too.
                                Debug.WriteLine("CertificateConfiguration = " + sectionProp.Value.ToString());
                                certificateConfiguration = JsonConvert.DeserializeObject<CertificateConfiguration>(sectionProp.Value.ToString());
                            }
                            break;
                        case "windowsUpdates":
                            {
                                Debug.WriteLine("windowsUpdates = " + sectionProp.Value.ToString());
                                var configuration = JsonConvert.DeserializeObject<SetWindowsUpdatesConfiguration>(sectionProp.Value.ToString());
                                await this._systemConfiguratorProxy.SendCommandAsync(new SetWindowsUpdatesRequest(configuration));
                            }
                            break;
                        case "startupApps":
                            {
                                Debug.WriteLine("startupApps = " + sectionProp.Value.ToString());
                                var startupApps = JsonConvert.DeserializeObject<StartupApps>(sectionProp.Value.ToString());
                                StartupAppInfo foregroundApp = new StartupAppInfo(startupApps.foreground, false /*!background*/);
                                await this._systemConfiguratorProxy.SendCommandAsync(new AddStartupAppRequest(foregroundApp));
                            }
                            break;
                        default:
                            // Not supported
                            break;
                    }
                }
            }

            // Now, handle the operations that depend on others in the necessary order.
            // By now, Azure storage information should have been captured.

            if (!String.IsNullOrEmpty(_externalStorageConnectionString))
            {
                if (certificateConfiguration != null)
                {
                    await ProcessDesiredCertificateConfigurationAsync(_systemConfiguratorProxy, _externalStorageConnectionString, "certificates", certificateConfiguration);
                }
            }
        }

        private async Task<Message.GetCertificateConfigurationResponse> GetCertificateConfigurationAsync()
        {
            var request = new Message.GetCertificateConfigurationRequest();
            return (await this._systemConfiguratorProxy.SendCommandAsync(request) as Message.GetCertificateConfigurationResponse);
        }

        private async Task<Message.GetWindowsUpdatesResponse> GetWindowsUpdatesAsync()
        {
            var request = new Message.GetWindowsUpdatesRequest();
            return (await this._systemConfiguratorProxy.SendCommandAsync(request) as Message.GetWindowsUpdatesResponse);
        }

        private async Task ReportAllDeviceProperties()
        {
            Logger.Log("Reporting all device properties to device twin...", LoggingLevel.Information);

            Logger.Log("Querying device state...", LoggingLevel.Information);

            Message.GetCertificateConfigurationResponse certificateConfigurationResponse = await GetCertificateConfigurationAsync();
            Message.GetWindowsUpdatesResponse windowsUpdatesResponse = await GetWindowsUpdatesAsync();

            JObject windowsObj = new JObject();
            foreach (var handler in this._desiredPropertyMap.Values)
            {
                // TODO: how do we ensure that only Reported=yes sections report results?
                windowsObj[handler.PropertySectionName] = await handler.GetReportedPropertyAsync();
            }

            // ToDo: Temporary fix.
            CertificatesDataContract.ReportedProperties reportedProperties = new CertificatesDataContract.ReportedProperties();
            reportedProperties.certificateStore_CA_System = certificateConfigurationResponse.configuration.certificateStore_CA_System;
            reportedProperties.certificateStore_My_System = certificateConfigurationResponse.configuration.certificateStore_My_System;
            reportedProperties.certificateStore_My_User = certificateConfigurationResponse.configuration.certificateStore_My_User;
            reportedProperties.certificateStore_Root_System = "<too long to store in device twin>"; // certificateConfigurationResponse.configuration.certificateStore_Root_System;
            reportedProperties.rootCATrustedCertificates_CA = certificateConfigurationResponse.configuration.rootCATrustedCertificates_CA;
            reportedProperties.rootCATrustedCertificates_Root = "<too long to store in device twin>"; ; // certificateConfigurationResponse.configuration.rootCATrustedCertificates_Root;
            reportedProperties.rootCATrustedCertificates_TrustedPeople = certificateConfigurationResponse.configuration.rootCATrustedCertificates_TrustedPeople;
            reportedProperties.rootCATrustedCertificates_TrustedPublisher = certificateConfigurationResponse.configuration.rootCATrustedCertificates_TrustedPublisher;

            // windowsObj["certificates"] = JObject.Parse(JsonConvert.SerializeObject(certificateConfigurationResponse));
            windowsObj["certificates"] = JObject.FromObject(reportedProperties);
            windowsObj["windowsUpdates"] = JObject.Parse(JsonConvert.SerializeObject(windowsUpdatesResponse));

            Dictionary<string, object> collection = new Dictionary<string, object>();
            collection[DMJSonConstants.DTWindowsIoTNameSpace] = windowsObj;

            Debug.WriteLine($"Report properties: {collection[DMJSonConstants.DTWindowsIoTNameSpace].ToString()}");
            _deviceTwin.ReportProperties(collection).FireAndForget();
        }

        private Task<string> ReportAllDevicePropertiesMethodHandler(string jsonParam)
        {
            Logger.Log("Handling direct method " + CommonDataContract.ReportAllAsync + " ...", LoggingLevel.Verbose);

            ReportAllDeviceProperties().FireAndForget();

            StatusSection status = new StatusSection(StatusSection.StateType.Pending);
            return Task.FromResult<string>(status.AsJsonObject().ToString());
        }

        // Data members
        JObject _desiredCache = new JObject();
        ISystemConfiguratorProxy _systemConfiguratorProxy;
        FactoryResetHandler _factoryResetHandler;
        WindowsUpdatePolicyHandler _windowsUpdatePolicyHandler;
        RebootCmdHandler _rebootCmdHandler;
        ExternalStorageHandler _externalStorageHandler;
        WindowsTelemetryHandler _windowsTelemetryHandler;
        IDeviceManagementRequestHandler _hostAppHandler;
        IDeviceTwin _deviceTwin;
        string _externalStorageConnectionString;
        Dictionary<string, IClientPropertyHandler> _desiredPropertyMap;
        Dictionary<string, List<IClientPropertyDependencyHandler>> _desiredPropertyDependencyMap;
        CommandStatus _lastCommandStatus = CommandStatus.NotStarted;
    }

}
