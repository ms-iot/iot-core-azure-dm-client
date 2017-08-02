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
using System.Collections.Generic;
using System.Windows.Controls;

namespace DMDashboard
{

    public class CollectorReportedState
    {
        public string Name { get; set; }
        public string TraceLogFileMode { get; set; }
        public int LogFileSizeLimitMB { get; set; }
        public string LogFileFolder { get; set; }
        public string Started { get; set; }

        public List<ProviderDesiredState> ProviderList { get; set; }

        public CollectorReportedState()
        {
            TraceLogFileMode = "sequential";
            LogFileSizeLimitMB = 4;
            LogFileFolder = "";
            Started = "yes";
        }

        public static CollectorReportedState FromJson(string collectorName, JToken jValueToken)
        {
            if (!(jValueToken is JObject))
            {
                return null;
            }

            CollectorReportedState collectorReportedState = new CollectorReportedState();

            JObject jValueObject = (JObject)jValueToken;

            collectorReportedState.Name = collectorName;

            foreach (JToken token in jValueObject.Children())
            {
                if (!(token is JProperty))
                {
                    continue;
                }
                JProperty jProperty = (JProperty)token;

                switch (jProperty.Name)
                {
                    case "traceLogFileMode":
                        collectorReportedState.TraceLogFileMode = (string)jProperty.Value;
                        break;
                    case "logFileSizeLimitMB":
                        collectorReportedState.LogFileSizeLimitMB = (int)jProperty.Value;
                        break;
                    case "logFileFolder":
                        collectorReportedState.LogFileFolder = (string)jProperty.Value;
                        break;
                    case "started":
                        collectorReportedState.Started = (string)jProperty.Value;
                        break;
                    default:
                        ProviderDesiredState providerReportedState = ProviderDesiredState.ProviderDesiredStateFromJson(jProperty.Name, jProperty.Value);
                        if (providerReportedState != null)
                        {
                            if (collectorReportedState.ProviderList == null)
                            {
                                collectorReportedState.ProviderList = new List<ProviderDesiredState>();
                            }
                            collectorReportedState.ProviderList.Add(providerReportedState);

                        }
                        break;
                }
            }

            return collectorReportedState;
        }
    }

    public partial class CollectorReportedStateControl : UserControl
    {
        public CollectorReportedStateControl()
        {
            InitializeComponent();
        }
    }
}
