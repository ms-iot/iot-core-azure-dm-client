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
using System;
using System.Collections.Generic;
using System.Text;

namespace Microsoft.Devices.Management.DMDataContract
{
    public class EventTracingDataContract
    {
        public const string SectionName = "eventTracingCollectors";

        public const string JsonLogFileFolder = "logFileFolder";
        public const string JsonLogFileName = "logFileName";
        public const string JsonLogFileSizeLimitMB = "logFileSizeLimitMB";
        public const string JsonStarted = "started";
        public const string JsonType = "type";
        public const string JsonProvider = "provider";
        public const string JsonKeywords = "keywords";
        public const string JsonEnabled = "enabled";

        public const string JsonReportLevelDetailed = "detailed";
        public const string JsonReportLevelMinimal = "minimal";
        public const string JsonReportLevelNone = "none";

        public const string JsonTraceLevel = "traceLevel";
        public const string JsonTraceLevelCritical = "critical";
        public const string JsonTraceLevelError = "error";
        public const string JsonTraceLevelWarning = "warning";
        public const string JsonTraceLevelInformation = "information";
        public const string JsonTraceLevelVerbose = "verbose";

        public const string JsonTraceMode = "traceLogFileMode";
        public const string JsonTraceModeSequential = "sequential";
        public const string JsonTraceModeCircular = "circular";

        public const string JsonDefaultTraceLevel = JsonTraceLevelCritical;

        public enum TraceLevel
        {
            Verbose,
            Information,
            Warning,
            Error,
            Critical
        }

        public enum TraceMode
        {
            Sequential,
            Circular
        }

        public enum ReportLevel
        {
            Detailed,
            Minimal,
            None
        }

        public static TraceLevel TraceLevelFromJsonString(string s)
        {
            switch (s)
            {
                case JsonTraceLevelCritical: return TraceLevel.Critical;
                case JsonTraceLevelError: return TraceLevel.Error;
                case JsonTraceLevelWarning: return TraceLevel.Warning;
                case JsonTraceLevelInformation: return TraceLevel.Information;
                case JsonTraceLevelVerbose: return TraceLevel.Verbose;
            }
            throw new Exception("Unknown TraceLevel: " + s);
        }

        public static string TraceLevelToJsonString(TraceLevel traceLevel)
        {
            switch (traceLevel)
            {
                case TraceLevel.Critical: return JsonTraceLevelCritical;
                case TraceLevel.Error: return JsonTraceLevelError;
                case TraceLevel.Warning: return JsonTraceLevelWarning;
                case TraceLevel.Information: return JsonTraceLevelInformation;
                case TraceLevel.Verbose: return JsonTraceLevelVerbose;
            }
            throw new Exception("Unknown TraceLevel: " + traceLevel.ToString());
        }

        public static TraceMode TraceModeFromJsonString(string s)
        {
            switch (s)
            {
                case JsonTraceModeSequential: return TraceMode.Sequential;
                case JsonTraceModeCircular: return TraceMode.Circular;
            }
            throw new Exception("Unknown TraceMode: " + s);
        }

        public static string TraceModeToJsonString(TraceMode traceMode)
        {
            switch (traceMode)
            {
                case TraceMode.Circular: return JsonTraceModeCircular;
                case TraceMode.Sequential: return JsonTraceModeSequential;
            }
            throw new Exception("Unknown TraceMode: " + traceMode);
        }

        public static ReportLevel ReportLevelFromJsonString(string s)
        {
            switch (s)
            {
                case JsonReportLevelDetailed: return ReportLevel.Detailed;
                case JsonReportLevelMinimal: return ReportLevel.Minimal;
                case JsonReportLevelNone: return ReportLevel.None;
            }
            throw new Exception("Unknown ReportLevel: " + s);
        }

        public class Provider
        {
            public string guid;
            public TraceLevel traceLevel;
            public string keywords;
            public bool enabled;

            public static Provider FromJsonObject(string guid, JObject obj)
            {
                Provider provider = new Provider();
                provider.guid = guid;
                provider.traceLevel = TraceLevelFromJsonString(Utils.GetString(obj, JsonTraceLevel, JsonDefaultTraceLevel));
                provider.keywords = Utils.GetString(obj, JsonKeywords, "");
                provider.enabled = Utils.GetBool(obj, JsonEnabled, false);
                return provider;
            }

            public JObject ToJsonObject()
            {
                JObject jObject = new JObject();
                jObject.Add(JsonTraceLevel, TraceLevelToJsonString(traceLevel));
                jObject.Add(JsonKeywords, keywords);
                jObject.Add(JsonEnabled, enabled);
                jObject.Add(JsonType, JsonProvider);
                return jObject;
            }
        }

        public class CollectorInner
        {
            public string name;
            public string logFileFolder;
            public string logFileName;
            public int logFileSizeLimitMB;
            public TraceMode traceMode;
            public bool started;
            public List<Provider> providers;

            public CollectorInner()
            {
                providers = new List<Provider>();
            }

            private static Provider TryReadProviderFromJson(JProperty cspProperty)
            {
                Guid providerGuid;
                if (!Guid.TryParse(cspProperty.Name, out providerGuid))
                {
                    return null;
                }

                if (!(cspProperty.Value is JObject))
                {
                    return null;
                }

                JObject jCSPProvider = (JObject)cspProperty.Value;
                if ((string)jCSPProvider.GetValue(JsonType) != JsonProvider)
                {
                    return null;
                }

                return Provider.FromJsonObject(cspProperty.Name, jCSPProvider);
            }

            public static CollectorInner FromJsonObject(JObject obj)
            {
                CollectorInner collector = new CollectorInner();

                foreach (JToken cspToken in obj.Children())
                {
                    if (!(cspToken is JProperty))
                    {
                        continue;
                    }

                    JProperty cspProperty = (JProperty)cspToken;
                    if (cspProperty.Name == JsonLogFileFolder)
                    {
                        collector.logFileFolder = (string)cspProperty.Value;
                    }
                    else if (cspProperty.Name == JsonLogFileName)
                    {
                        collector.logFileName = (string)cspProperty.Value;
                    }
                    else if (cspProperty.Name == JsonLogFileSizeLimitMB)
                    {
                        collector.logFileSizeLimitMB = (int)cspProperty.Value;
                    }
                    else if (cspProperty.Name == JsonTraceMode)
                    {
                        collector.traceMode = TraceModeFromJsonString((string)cspProperty.Value);
                    }
                    else if (cspProperty.Name == JsonStarted)
                    {
                        collector.started = (bool)cspProperty.Value;
                    }
                    else
                    {
                        Provider provider = TryReadProviderFromJson(cspProperty);
                        if (provider != null)
                        {
                            collector.providers.Add(provider);
                        }
                        // We don't throw errors here because we only read the node
                        // we know about. Extra nodes are allowed.
                    }
                }

                return collector;
            }

            public JObject ToJsonObject()
            {
                JObject jObject = new JObject();

                jObject.Add(JsonLogFileFolder, logFileFolder);
                jObject.Add(JsonLogFileName, logFileName);
                jObject.Add(JsonLogFileSizeLimitMB, logFileSizeLimitMB);
                jObject.Add(JsonTraceMode, TraceModeToJsonString(traceMode));
                jObject.Add(JsonStarted, started);

                foreach (Provider provider in providers)
                {
                    jObject.Add(provider.guid, provider.ToJsonObject());
                }

                return jObject;
            }
        }

        public class CollectorDesiredState
        {
            public string name;
            public bool reportToDeviceTwin;
            public bool applyFromDeviceTwin;
            public CollectorInner collectorInner;

            public static CollectorDesiredState FromJsonObject(string name, JObject obj)
            {
                CollectorDesiredState collector = new CollectorDesiredState();
                collector.name = name;
                collector.reportToDeviceTwin = Utils.GetString(obj, CommonDataContract.JsonReportProperties, CommonDataContract.JsonNoString) == CommonDataContract.JsonYesString;

                JToken jApplyProperties = Utils.GetJToken(obj, CommonDataContract.JsonApplyProperties);
                if (jApplyProperties is JObject)
                {
                    collector.applyFromDeviceTwin = true;
                    collector.collectorInner = CollectorInner.FromJsonObject((JObject)jApplyProperties);
                }
                else
                {
                    collector.applyFromDeviceTwin = false;
                }

                return collector;
            }
        }

        public class DesiredProperties
        {
            public DesiredProperties()
            {
                generalReportLevel = ReportLevel.Minimal;
                collectors = new List<CollectorDesiredState>();
            }

            public static DesiredProperties FromJsonObject(JObject timeServiceObject)
            {
                DesiredProperties desiredProperties = new DesiredProperties();

                foreach (JToken jToken in timeServiceObject.Children())
                {
                    if (!(jToken is JProperty))
                    {
                        continue;
                    }
                    JProperty collectorNode = (JProperty)jToken;

                    // Check if it is query ("?")...
                    if (collectorNode.Name == CommonDataContract.JsonQuery &&
                        collectorNode.Value is JValue && collectorNode.Value.Type == JTokenType.String)
                    {
                        desiredProperties.generalReportLevel = ReportLevelFromJsonString(collectorNode.Value.ToString());
                    }
                    else if (collectorNode.Value is JObject)
                    {
                        desiredProperties.collectors.Add(CollectorDesiredState.FromJsonObject(collectorNode.Name, (JObject)collectorNode.Value));
                    }
                }

                return desiredProperties;
            }

            public string ToJsonString()
            {
                StringBuilder sb = new StringBuilder();

                return sb.ToString();
            }

            public ReportLevel generalReportLevel;
            public List<CollectorDesiredState> collectors;
        }

        public class ReportedProperties
        {
            public ReportedProperties()
            {
                collectorsDetailed = new List<CollectorInner>();
                collectorsMinimal = new List<string>();
            }

            public static ReportedProperties FromJsonObject(JObject json)
            {
                ReportedProperties reportedProperties = new ReportedProperties();

                return reportedProperties;
            }

            public JObject ToJsonObject()
            {
                JObject obj = new JObject();

                foreach (CollectorInner collector in collectorsDetailed)
                {
                    obj.Add(collector.name, collector.ToJsonObject());
                }

                foreach (string collectorsMinimal in collectorsMinimal)
                {
                    obj.Add(collectorsMinimal, new JValue(""));
                }

                return obj;
            }

            public List<CollectorInner> collectorsDetailed;
            public List<string> collectorsMinimal;
        }
    }
}