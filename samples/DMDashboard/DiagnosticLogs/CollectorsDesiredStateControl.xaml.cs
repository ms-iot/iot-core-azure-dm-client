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
using System.Windows;
using System.Windows.Controls;

namespace DMDashboard
{
    public partial class CollectorsDesiredStateControl : UserControl
    {
        const string JsonDetailed = "detailed";
        const string JsonMinimal = "minimal";
        const string JsonNone = "none";

        public string SectionName
        {
            get
            {
                return "eventTracingCollectors";
            }
        }

        public List<CollectorDesiredState> CollectorsConfigurations
        {
            set
            {
                ControlList.Children.Clear();
                for (int i = 0; i < value.Count; ++i)
                {
                    CollectorDesiredStateControl collectorDesiredStateControl = new CollectorDesiredStateControl();
                    collectorDesiredStateControl.DataContext = value[i];

                    foreach (ProviderDesiredState provider in value[i].ProviderList)
                    {
                        ProviderDesiredStateControl providerControl = new ProviderDesiredStateControl();
                        providerControl.DataContext = provider;
                        collectorDesiredStateControl.ProviderList.Add(providerControl);
                    }

                    ControlList.Children.Add(collectorDesiredStateControl);
                }
            }
        }

        public CollectorsDesiredStateControl()
        {
            InitializeComponent();
        }

        private void OnAdd(object sender, RoutedEventArgs e)
        {
            CollectorDesiredStateControl collectorDesiredStateControl = new CollectorDesiredStateControl();
            collectorDesiredStateControl.DataContext = new CollectorDesiredState();
            ControlList.Children.Add(collectorDesiredStateControl);
        }

        public string ToJson()
        {
            StringBuilder sb = new StringBuilder();

            switch (NonDeviceTwinCollectors.SelectedIndex)
            {
                case 0: // detailed
                    sb.Append("\"?\" : \"" + JsonDetailed + "\"\n");
                    break;
                case 1: // minimal
                    sb.Append("\"?\" : \"" + JsonMinimal + "\"\n");
                    break;
                case 2: // none
                    sb.Append("\"?\" : \"" + JsonNone + "\"\n");
                    break;
            }

            foreach (CollectorDesiredStateControl control in ControlList.Children)
            {
                string s = control.ToJsonString();
                if (!String.IsNullOrEmpty(s))
                {
                    if (sb.Length > 0)
                    {
                        sb.Append(",\n");
                    }
                    sb.Append(s);
                }
            }

            if (sb.Length == 0)
            {
                return "\"" + SectionName + "\" : null";
            }

            return "\"" + SectionName + "\" : {\n" + sb.ToString() + "\n}";
        }

        public void FromJson(JObject jRoot)
        {
            JToken collectorsToken = jRoot.SelectToken("properties.desired.windows." + SectionName);
            if (collectorsToken == null || !(collectorsToken is JObject))
            {
                return;
            }
            JObject collectorsObject = (JObject)collectorsToken;

            List<CollectorDesiredState> collectors = new List<CollectorDesiredState>();
            foreach (JProperty property in collectorsObject.Children())
            {
                if (property.Name == "?")
                {
                    if (property.Value is JValue && property.Value.Type == JTokenType.String)
                    {
                        switch ((string)property.Value)
                        {
                            case JsonDetailed:
                                NonDeviceTwinCollectors.SelectedIndex = 0;
                                break;
                            case JsonMinimal:
                                NonDeviceTwinCollectors.SelectedIndex = 1;
                                break;
                            case JsonNone:
                                NonDeviceTwinCollectors.SelectedIndex = 2;
                                break;
                        }
                    }
                }
                else
                {
                    CollectorDesiredState collectorDesiredState = CollectorDesiredState.FromJson(property.Name, property.Value);
                    if (collectorDesiredState != null)
                    {
                        collectors.Add(collectorDesiredState);
                    }
                }
            }
            CollectorsConfigurations = collectors;
        }
    }
}
