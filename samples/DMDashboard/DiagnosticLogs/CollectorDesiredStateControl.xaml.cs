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

using DMDataContract;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Globalization;
using System.Windows.Controls;
using System.Windows.Data;

namespace DMDashboard
{
    class FileModeToInt : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            if (!(value is string))
                return 0;

            return (string)value == "sequential" ? 0 : 1;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            if (!(value is int))
                return "sequential";

            return (int)value == 1 ? "circular" : "sequential";
        }
    }

    public class CollectorDesiredState
    {
        public string Name { get; set; }
        public string ReportProperties { get; set; }
        public string ApplyProperties { get; set; }
        public string TraceLogFileMode { get; set; }
        public int LogFileSizeLimitMB { get; set; }
        public string LogFileFolder { get; set; }
        public string Started { get; set; }

        public List<ProviderDesiredState> ProviderList { get; set; }

        public CollectorDesiredState()
        {
            ReportProperties = "yes";
            ApplyProperties = "yes";
            TraceLogFileMode = "sequential";
            LogFileSizeLimitMB = 4;
            LogFileFolder = "";
            Started = "yes";
        }

        public static CollectorDesiredState FromJson(string collectorName, JToken jValueToken)
        {
            if (!(jValueToken is JObject))
            {
                return null;
            }
            JObject jValueObject = (JObject)jValueToken;

            CollectorDesiredState collectorDesiredState = new CollectorDesiredState();

            collectorDesiredState.Name = collectorName;
            collectorDesiredState.ReportProperties = Utils.GetString(jValueObject, "reportProperties", "yes");
            JToken jApplyProperties = Utils.GetJToken(jValueObject, "applyProperties");
            if (jApplyProperties is JValue)
            {
                collectorDesiredState.ApplyProperties = (string)jApplyProperties;
            }
            else if (jApplyProperties is JObject)
            {
                collectorDesiredState.ApplyProperties = "yes";

                JObject jApplyPropertiesObject = (JObject)jApplyProperties;
                foreach (JToken token in jApplyPropertiesObject.Children())
                {
                    if (!(token is JProperty))
                    {
                        continue;
                    }
                    JProperty jProperty = (JProperty)token;

                    switch(jProperty.Name)
                    {
                        case "traceLogFileMode":
                            collectorDesiredState.TraceLogFileMode = (string)jProperty.Value;
                            break;
                        case "logFileSizeLimitMB":
                            collectorDesiredState.LogFileSizeLimitMB = (int)jProperty.Value;
                            break;
                        case "logFileFolder":
                            collectorDesiredState.LogFileFolder = (string)jProperty.Value;
                            break;
                        case "started":
                            collectorDesiredState.Started = (string)jProperty.Value;
                            break;
                        default:
                            ProviderDesiredState providerDesiredState = ProviderDesiredState.ProviderDesiredStateFromJson(jProperty.Name, jProperty.Value);
                            if (providerDesiredState != null)
                            {
                                if (collectorDesiredState.ProviderList == null)
                                {
                                    collectorDesiredState.ProviderList = new List<ProviderDesiredState>();
                                }
                                collectorDesiredState.ProviderList.Add(providerDesiredState);

                            }
                            break;
                    }
                }
            }

            return collectorDesiredState;
        }
    }

    public partial class CollectorDesiredStateControl : DMSectionControl
    {
        public CollectorDesiredStateControl()
        {
            InitializeComponent();
        }

        public override UIElementCollection PropertiesCollection
        {
            get
            {
                return PropertiesControls.Children;
            }
        }

        public override bool ApplyProperties
        {
            get
            {
                return ApplyPropertiesCombo.PropertyValue == DMJSonConstants.YesString;
            }
        }

        public override bool ReportProperties
        {
            get
            {
                return ReportPropertiesCombo.PropertyValue == DMJSonConstants.YesString;
            }
        }

        public override string SectionName
        {
            get
            {
                return CollectorName.PropertyValue;
            }
            set
            {
                CollectorName.PropertyValue = value;
            }
        }
    }
}
