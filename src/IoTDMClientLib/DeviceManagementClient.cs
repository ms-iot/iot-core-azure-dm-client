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
using System.Diagnostics;
using System.Linq;
using System.Threading.Tasks;
using System.Threading;
using Windows.Foundation.Diagnostics;

namespace Microsoft.Devices.Management
{
    public class DeviceManagementClient : IClientHandlerCallBack
    {
        const string MethodGetCertificateDetails = DMJSonConstants.DTWindowsIoTNameSpace + ".getCertificateDetails";

        public IDeviceTwin DeviceTwin { get { return _deviceTwin; } }

        private DeviceManagementClient(IDeviceTwin deviceTwin, IDeviceManagementRequestHandler hostAppHandler, ISystemConfiguratorProxy systemConfiguratorProxy)
        {
            Logger.Log("Entering DeviceManagementClient constructor.", LoggingLevel.Verbose);

            this._lastDesiredPropertyVersion = -1;
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

            // Create/Attach handlers...
            deviceManagementClient._externalStorageHandler = new ExternalStorageHandler(clientCallback, systemConfiguratorProxy);
            deviceManagementClient.AddPropertyHandler(deviceManagementClient._externalStorageHandler);

            var deviceHealthAttestationHandler = new DeviceHealthAttestationHandler(clientCallback, systemConfiguratorProxy);
            deviceManagementClient.AddPropertyHandler(deviceHealthAttestationHandler);
            await deviceManagementClient.AddDirectMethodHandlerAsync(deviceHealthAttestationHandler);

            deviceManagementClient._factoryResetHandler = new FactoryResetHandler(clientCallback, systemConfiguratorProxy);
            await deviceManagementClient.AddDirectMethodHandlerAsync(deviceManagementClient._factoryResetHandler);

            deviceManagementClient._windowsUpdatePolicyHandler = new WindowsUpdatePolicyHandler(clientCallback, systemConfiguratorProxy);
            deviceManagementClient.AddPropertyHandler(deviceManagementClient._windowsUpdatePolicyHandler);

            var windowsUpdatesHandler = new WindowsUpdatesHandler(clientCallback, systemConfiguratorProxy);
            deviceManagementClient.AddPropertyHandler(windowsUpdatesHandler);

            var wifiHandler = new WifiHandler(clientCallback, systemConfiguratorProxy);
            deviceManagementClient.AddPropertyHandler(wifiHandler);
            await deviceManagementClient.AddDirectMethodHandlerAsync(wifiHandler);

            var appxHandler = new AppxManagement(clientCallback, systemConfiguratorProxy, deviceManagementClient._desiredCache);
            deviceManagementClient.AddPropertyHandler(appxHandler);

            var appxLifeCycleHandler = new AppxLifeCycleHandler(clientCallback, systemConfiguratorProxy);
            await deviceManagementClient.AddDirectMethodHandlerAsync(appxLifeCycleHandler);

            var eventTracingHandler = new EventTracingHandler(clientCallback, systemConfiguratorProxy, deviceManagementClient._desiredCache);
            deviceManagementClient.AddPropertyHandler(eventTracingHandler);

            var storageHandler = new StorageHandler(clientCallback, systemConfiguratorProxy);
            await deviceManagementClient.AddDirectMethodHandlerAsync(storageHandler);

            var timeSettingsHandler = new TimeSettingsHandler(clientCallback, systemConfiguratorProxy);
            deviceManagementClient.AddPropertyHandler(timeSettingsHandler);

            deviceManagementClient._timeServiceHandler = new TimeServiceHandler(clientCallback, systemConfiguratorProxy);
            deviceManagementClient.AddPropertyHandler(deviceManagementClient._timeServiceHandler);

            deviceManagementClient._rebootCmdHandler = new RebootCmdHandler(
                clientCallback,
                systemConfiguratorProxy,
                deviceManagementClient._hostAppHandler,
                deviceManagementClient._windowsUpdatePolicyHandler);
            await deviceManagementClient.AddDirectMethodHandlerAsync(deviceManagementClient._rebootCmdHandler);

            var rebootInfoHandler = new RebootInfoHandler(clientCallback, systemConfiguratorProxy, deviceManagementClient._desiredCache);
            deviceManagementClient.AddPropertyHandler(rebootInfoHandler);

            deviceManagementClient._windowsTelemetryHandler = new WindowsTelemetryHandler(clientCallback, systemConfiguratorProxy);
            deviceManagementClient.AddPropertyHandler(deviceManagementClient._windowsTelemetryHandler);

            var deviceInfoHandler = new DeviceInfoHandler(clientCallback, systemConfiguratorProxy);
            deviceManagementClient.AddPropertyHandler(deviceInfoHandler);

            var dmAppStoreUpdate = new DmAppStoreUpdateHandler(clientCallback, systemConfiguratorProxy);
            await deviceManagementClient.AddDirectMethodHandlerAsync(dmAppStoreUpdate);

            deviceManagementClient._certificateHandler = new CertificateHandler(clientCallback, systemConfiguratorProxy);
            deviceManagementClient.AddPropertyHandler(deviceManagementClient._certificateHandler);
            await deviceTwin.SetMethodHandlerAsync(MethodGetCertificateDetails, deviceManagementClient._certificateHandler.GetCertificateDetailsHandlerAsync);

            return deviceManagementClient;
        }

        internal static DeviceManagementClient Create(IDeviceTwin deviceTwin, IDeviceManagementRequestHandler requestHandler, ISystemConfiguratorProxy systemConfiguratorProxy)
        {
            return new DeviceManagementClient(deviceTwin, requestHandler, systemConfiguratorProxy);
        }

        public void SignalOperationComplete()
        {
            _deviceTwin.SignalOperationComplete();
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

        public void ExtractInfoFromDesiredProperties(Dictionary<string, object> desiredProperties, out long version, out JObject windowsProperties)
        {
            windowsProperties = null;
            version = -1;

            object versionValue = null;
            if (desiredProperties.TryGetValue(DMJSonConstants.DTVersionString, out versionValue) && versionValue != null && versionValue is long)
            {
                version = (long)versionValue;
            }

            object windowsPropValue = null;
            if (desiredProperties.TryGetValue(DMJSonConstants.DTWindowsIoTNameSpace, out windowsPropValue) && windowsPropValue != null && windowsPropValue is JObject)
            {
                windowsProperties = (JObject)windowsPropValue;
            }
        }

        public async Task ApplyDesiredStateAsync()
        {
            Logger.Log("Retrieving desired state from device twin...", LoggingLevel.Verbose);
            await ApplyDesiredStateAsync(-1, new JObject(), true);
        }

        public async Task ApplyDesiredStateAsync(Dictionary<string, object> desiredProperties)
        {
            Logger.Log("Applying desired state...", LoggingLevel.Verbose);

            try
            {
                JObject windowsProps = null;
                long version = 0;

                object windowsPropsObj = null;
                if (desiredProperties.TryGetValue(DMJSonConstants.DTWindowsIoTNameSpace, out windowsPropsObj))
                {
                    windowsProps = (JObject)windowsPropsObj;
                }
                else
                {
                    // Nothing to process for us.
                    return;
                }

                object versionObj = null;
                if (desiredProperties.TryGetValue(DMJSonConstants.DTVersionString, out versionObj))
                {
                    version = (long)versionObj;
                }

                await ApplyDesiredStateAsync(version, windowsProps, false);
            }
            catch (Exception)
            {
                Debug.WriteLine("No properties.desired." + DMJSonConstants.DTWindowsIoTNameSpace + " is found.");
            }
        }

        public async Task RebootAsync()
        {
            await _rebootCmdHandler.RebootAsync();
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

        public async Task SetTimeServiceAsync(TimeServiceState desiredState)
        {
            await _timeServiceHandler.SetTimeServiceAsync(desiredState);
        }

        public async Task<TimeServiceState> GetTimeServiceStateAsync()
        {
            return await _timeServiceHandler.GetTimeServiceStateAsync();
        }

        public async Task SetWindowsUpdateRingAsync(WindowsUpdateRingState state)
        {
            await _windowsUpdatePolicyHandler.SetRingAsync(state);
        }

        public async Task<WindowsUpdateRingState> GetWindowsUpdateRingAsync()
        {
            return await _windowsUpdatePolicyHandler.GetRingAsync();
        }

        public async Task AllowReboots(bool allowReboots)
        {
            await _rebootCmdHandler.AllowReboots(allowReboots);
        }

        private async Task ApplyDesiredStateAsync(long version, JObject windowsPropValue, bool forceFullTwinUpdate)
        {
            Logger.Log(string.Format("Applying {0} node desired state for version {1} ...", DMJSonConstants.DTWindowsIoTNameSpace, version), LoggingLevel.Verbose);

            try
            {
                // Only one set of updates at a time...
                await _desiredPropertiesLock.WaitAsync();

                // If we force a full twin update OR 
                //    we haven't processed any updates yet OR
                //    we missed a version ...
                // Then, get all of the twin's properties
                if (forceFullTwinUpdate || _lastDesiredPropertyVersion == -1 || version > (_lastDesiredPropertyVersion + 1))
                {
                    Dictionary<string, object> desiredProperties = await this._deviceTwin.GetDesiredPropertiesAsync();
                    ExtractInfoFromDesiredProperties(desiredProperties, out version, out windowsPropValue);
                }
                else if (version <= _lastDesiredPropertyVersion)
                {
                    // If this version is older (or the same) than the last we processed ...
                    // Then, skip this update
                    return;
                }
                _lastDesiredPropertyVersion = version;
                if (windowsPropValue == null)
                {
                    // Nothing to apply.
                    return;
                }

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
                }
            }
            finally
            {
                this._desiredPropertiesLock.Release();
            }
        }

        private async Task ReportAllDeviceProperties()
        {
            Logger.Log("Reporting all device properties to device twin...", LoggingLevel.Information);

            Logger.Log("Querying device state...", LoggingLevel.Information);

            JObject windowsObj = new JObject();
            foreach (var handler in this._desiredPropertyMap.Values)
            {
                StatusSection statusSection = new StatusSection(StatusSection.StateType.Pending);
                await ReportStatusAsync(handler.PropertySectionName, statusSection);

                // TODO: how do we ensure that only Reported=yes sections report results?
                try
                {
                    windowsObj[handler.PropertySectionName] = await handler.GetReportedPropertyAsync();

                    statusSection.State = StatusSection.StateType.Completed;
                    Logger.Log(statusSection.ToString(), LoggingLevel.Information);
                    await ReportStatusAsync(handler.PropertySectionName, statusSection);
                }
                catch (Error e)
                {
                    statusSection.State = StatusSection.StateType.Failed;
                    statusSection.TheError = e;

                    Logger.Log(statusSection.ToString(), LoggingLevel.Error);
                    await ReportStatusAsync(handler.PropertySectionName, statusSection);
                }
                catch (Exception e)
                {
                    statusSection.State = StatusSection.StateType.Failed;
                    statusSection.TheError = new Error(ErrorSubSystem.Unknown, e.HResult, e.Message);

                    Logger.Log(statusSection.ToString(), LoggingLevel.Error);
                    await ReportStatusAsync(handler.PropertySectionName, statusSection);
                }
            }

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
        CertificateHandler _certificateHandler;
        FactoryResetHandler _factoryResetHandler;
        WindowsUpdatePolicyHandler _windowsUpdatePolicyHandler;
        RebootCmdHandler _rebootCmdHandler;
        ExternalStorageHandler _externalStorageHandler;
        TimeServiceHandler _timeServiceHandler;
        WindowsTelemetryHandler _windowsTelemetryHandler;
        IDeviceManagementRequestHandler _hostAppHandler;
        IDeviceTwin _deviceTwin;
        Dictionary<string, IClientPropertyHandler> _desiredPropertyMap;
        Dictionary<string, List<IClientPropertyDependencyHandler>> _desiredPropertyDependencyMap;
        CommandStatus _lastCommandStatus = CommandStatus.NotStarted;
        long _lastDesiredPropertyVersion = -1;
        SemaphoreSlim _desiredPropertiesLock = new SemaphoreSlim(1);
    }
}
