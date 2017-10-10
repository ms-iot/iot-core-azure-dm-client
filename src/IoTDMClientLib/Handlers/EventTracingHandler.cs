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
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using Microsoft.Devices.Management.Message;
using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using Windows.Data.Json;
using Windows.Foundation.Diagnostics;

namespace Microsoft.Devices.Management
{
    static class SystemConfiguratorConverters
    {
        private static ProviderConfiguration ProviderToMessage(EventTracingDataContract.Provider provider)
        {
            ProviderConfiguration msgProvider = new ProviderConfiguration();
            msgProvider.Guid = provider.guid;
            msgProvider.TraceLevel = EventTracingDataContract.TraceLevelToJsonString(provider.traceLevel);
            msgProvider.Keywords = provider.keywords;
            msgProvider.Enabled = provider.enabled;
            return msgProvider;
        }

        private static EventTracingDataContract.Provider ProviderFromMessage(ProviderConfiguration msgProvider)
        {
            EventTracingDataContract.Provider provider = new EventTracingDataContract.Provider();
            provider.enabled = msgProvider.Enabled;
            provider.guid = msgProvider.Guid;
            provider.keywords = msgProvider.Keywords;
            provider.traceLevel = EventTracingDataContract.TraceLevelFromJsonString(msgProvider.TraceLevel);
            return provider;
        }

        private static CollectorCSPConfiguration CollectorInternalToMessage(EventTracingDataContract.CollectorInner collector)
        {
            Logger.Log("Reading collector CSP desired configuration...", LoggingLevel.Information);

            CollectorCSPConfiguration cspConfiguration = new CollectorCSPConfiguration();
            cspConfiguration.LogFileFolder = collector.logFileFolder;
            cspConfiguration.LogFileName = collector.logFileName;
            cspConfiguration.LogFileSizeLimitMB = collector.logFileSizeLimitMB;
            cspConfiguration.TraceLogFileMode = EventTracingDataContract.TraceModeToJsonString(collector.traceMode);
            cspConfiguration.Started = collector.started;
            foreach (EventTracingDataContract.Provider provider in collector.providers)
            {
                ProviderConfiguration msgProvider = ProviderToMessage(provider);
            }
            return cspConfiguration;
        }

        public static CollectorDesiredConfiguration CollectorToMessage(EventTracingDataContract.CollectorDesiredState collector)
        {
            Logger.Log("Reading collector desired configuration...", LoggingLevel.Information);

            CollectorDesiredConfiguration msgCollector = new CollectorDesiredConfiguration();
            msgCollector.Name = collector.name;
            msgCollector.ReportToDeviceTwin = CommonDataContract.BooleanToYesNoJsonString(collector.reportToDeviceTwin);
            msgCollector.ApplyFromDeviceTwin = CommonDataContract.BooleanToYesNoJsonString(collector.applyFromDeviceTwin);
            msgCollector.CSPConfiguration = CollectorInternalToMessage(collector.collectorInner);
            return msgCollector;
        }

        public static EventTracingDataContract.CollectorInner CollectorInnerFromMessage(CollectorReportedConfiguration msgCollector)
        {
            EventTracingDataContract.CollectorInner collectorInner = new EventTracingDataContract.CollectorInner();

            collectorInner.name = msgCollector.Name;
            collectorInner.logFileFolder = msgCollector.CSPConfiguration.LogFileFolder;
            collectorInner.logFileName = msgCollector.CSPConfiguration.LogFileName;
            collectorInner.logFileSizeLimitMB = msgCollector.CSPConfiguration.LogFileSizeLimitMB;
            collectorInner.started = msgCollector.CSPConfiguration.Started;

            foreach (ProviderConfiguration msgProvider in msgCollector.CSPConfiguration.Providers)
            {
                collectorInner.providers.Add(ProviderFromMessage(msgProvider));
            }

            return collectorInner;
        }
    }

    class EventTracingHandler : IClientPropertyHandler
    {
        class EventTracingDesiredState
        {
            public static EventTracingDesiredState FromJsonObject(JObject eventTracingCollectorsNode)
            {
                Logger.Log("Building native request from desired json...", LoggingLevel.Information);

                EventTracingDataContract.DesiredProperties desiredPropertyies = EventTracingDataContract.DesiredProperties.FromJsonObject(eventTracingCollectorsNode);

                EventTracingDesiredState jsonRequest = new EventTracingDesiredState();
                jsonRequest.generalReportLevel = desiredPropertyies.generalReportLevel;
                jsonRequest.configuredCollectors = new HashSet<string>();
                jsonRequest.request = new SetEventTracingConfigurationRequest();

                foreach (EventTracingDataContract.CollectorDesiredState collector in desiredPropertyies.collectors)
                {
                    CollectorDesiredConfiguration msgCollector = SystemConfiguratorConverters.CollectorToMessage(collector);
                    jsonRequest.request.Collectors.Add(msgCollector);
                    jsonRequest.configuredCollectors.Add(msgCollector.Name);
                }

                return jsonRequest;
            }

            public SetEventTracingConfigurationRequest request { get; private set; }
            public EventTracingDataContract.ReportLevel generalReportLevel { get; private set; }
            public HashSet<string> configuredCollectors { get; private set; }
        }

        public EventTracingHandler(IClientHandlerCallBack callback, ISystemConfiguratorProxy systemConfiguratorProxy, JObject desiredCache)
        {
            _systemConfiguratorProxy = systemConfiguratorProxy;
            _callback = callback;
            _desiredCache = desiredCache;
        }

        // IClientPropertyHandler
        public string PropertySectionName
        {
            get
            {
                return EventTracingDataContract.SectionName;
            }
        }

        private async Task NullifyReported()
        {
            Logger.Log("Nullify " + PropertySectionName + "...", LoggingLevel.Information);
            await _callback.ReportPropertiesAsync(PropertySectionName, new JValue(CommonDataContract.JsonRefreshing));
        }

        private async Task<EventTracingDesiredState> SetEventTracingConfiguration(JObject desiredValue)
        {
            var jsonRequest = EventTracingDesiredState.FromJsonObject((JObject)desiredValue);

            // Send the request...
            if (jsonRequest.request.Collectors.Count != 0)
            {
                Logger.Log("Sending request...", LoggingLevel.Information);

                var setResponse = await _systemConfiguratorProxy.SendCommandAsync(jsonRequest.request) as Message.StringResponse;
                if (setResponse.Status != ResponseStatus.Success)
                {
                    throw new Error((int)setResponse.Status, setResponse.Response);
                }
            }
            return jsonRequest;
        }

        private JObject ReportedFromResponse(GetEventTracingConfigurationResponse response, EventTracingDataContract.ReportLevel generalReportLevel, HashSet<string> configuredCollectors)
        {
            Logger.Log("Building diagnostic logs reported properties...", LoggingLevel.Information);

            EventTracingDataContract.ReportedProperties reportedProperties = new EventTracingDataContract.ReportedProperties();

            foreach (CollectorReportedConfiguration msgCollector in response.Collectors)
            {
                string applicableReporting = EventTracingDataContract.JsonReportLevelNone;

                if (msgCollector.ReportToDeviceTwin == DMJSonConstants.YesString)
                {
                    applicableReporting = EventTracingDataContract.JsonReportLevelDetailed;
                }
                else if (!configuredCollectors.Contains(msgCollector.Name))
                { 
                    if (generalReportLevel == EventTracingDataContract.ReportLevel.Detailed)
                    {
                        applicableReporting = EventTracingDataContract.JsonReportLevelDetailed;
                    }
                    if (generalReportLevel == EventTracingDataContract.ReportLevel.Minimal)
                    {
                        applicableReporting = EventTracingDataContract.JsonReportLevelMinimal;
                    }
                }

                if (applicableReporting == EventTracingDataContract.JsonReportLevelDetailed)
                {
                    var collectorInner = SystemConfiguratorConverters.CollectorInnerFromMessage(msgCollector);
                    reportedProperties.collectorsDetailed.Add(collectorInner);
                }
                else if (applicableReporting == EventTracingDataContract.JsonReportLevelMinimal)
                {
                    reportedProperties.collectorsMinimal.Add(msgCollector.Name);
                }
            }

            return reportedProperties.ToJsonObject();
        }

        private async Task<JObject> GetEventTracingConfiguration(EventTracingDataContract.ReportLevel generalReportLevel, HashSet<string> configuredCollectors)
        {
            Logger.Log("Retrieving diagnostic logs configurations...", LoggingLevel.Information);

            GetEventTracingConfigurationRequest getRequest = new GetEventTracingConfigurationRequest();
            var getResponse = await _systemConfiguratorProxy.SendCommandAsync(getRequest);
            if (getResponse.Status != ResponseStatus.Success)
            {
                var stringResponse = getResponse as StringResponse;
                throw new Error((int)stringResponse.Status, stringResponse.Response);
            }

            return ReportedFromResponse(getResponse as GetEventTracingConfigurationResponse, generalReportLevel, configuredCollectors);
        }

        private async Task OnDesiredPropertyChangeAsync(JToken desiredValue)
        {
            Logger.Log("Processing desired properties for section: " + PropertySectionName + "...", LoggingLevel.Information);

            if (!(desiredValue is JObject))
            {
                return;
            }

            EventTracingDesiredState jsonRequest = await SetEventTracingConfiguration((JObject)desiredValue);
            _generalReportLevel = jsonRequest.generalReportLevel;
            _configuredCollectors = jsonRequest.configuredCollectors;

            JObject reportedProperties = await GetEventTracingConfiguration(_generalReportLevel, _configuredCollectors);

            await NullifyReported();

            await _callback.ReportPropertiesAsync(EventTracingDataContract.SectionName, reportedProperties);
        }

        private void UpdateCache(JToken desiredValue)
        {
            // Removal (with or without caching/merging) requires explicitly specified state.
            JToken cachedToken = _desiredCache.SelectToken(EventTracingDataContract.SectionName);
            if (cachedToken != null)
            {
                if (cachedToken is JObject)
                {
                    JObject cachedObject = (JObject)cachedToken;
                    cachedObject.Merge(desiredValue);
                }
            }
            else
            {
                _desiredCache[EventTracingDataContract.SectionName] = desiredValue;
            }
        }

        // IClientPropertyHandler
        public async Task<CommandStatus> OnDesiredPropertyChange(JToken desiredValue)
        {
            UpdateCache(desiredValue);

            // Need to revisit all the desired nodes (not only the changed ones) 
            // so that we can re-construct the correct reported list.
            await OnDesiredPropertyChangeAsync(_desiredCache[EventTracingDataContract.SectionName]);

            return CommandStatus.Committed;
        }

        // IClientPropertyHandler
        public async Task<JObject> GetReportedPropertyAsync()
        {
            return await GetEventTracingConfiguration(_generalReportLevel, _configuredCollectors);
        }

        private ISystemConfiguratorProxy _systemConfiguratorProxy;
        private IClientHandlerCallBack _callback;
        private JObject _desiredCache;
        private EventTracingDataContract.ReportLevel _generalReportLevel;
        private HashSet<string> _configuredCollectors;
    }
}
