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
    public partial class CollectorsReportedStateControl : UserControl
    {
        public string SectionName
        {
            get
            {
                return "eventTracingCollectors";
            }
        }

        public List<CollectorReportedState> CollectorsConfigurations
        {
            set
            {
                ControlList.Children.Clear();
                for (int i = 0; i < value.Count; ++i)
                {
                    CollectorReportedStateControl collectorReportedStateControl = new CollectorReportedStateControl();
                    collectorReportedStateControl.DataContext = value[i];

                    if (value[i].ProviderList != null)
                    {
                        foreach (ProviderDesiredState provider in value[i].ProviderList)
                        {
                            ProviderReportedStateControl providerControl = new ProviderReportedStateControl();
                            providerControl.DataContext = provider;
                            collectorReportedStateControl.ProviderList.Add(providerControl);
                        }
                    }

                    ControlList.Children.Add(collectorReportedStateControl);
                }
            }
        }

        public CollectorsReportedStateControl()
        {
            InitializeComponent();
        }

        public void FromJson(JObject jRoot)
        {
            List<CollectorReportedState> collectors = new List<CollectorReportedState>();
            foreach (JProperty property in jRoot.Children())
            {
                CollectorReportedState collectorReportedState = CollectorReportedState.FromJson(property.Name, property.Value);
                if (collectorReportedState != null)
                {
                    collectors.Add(collectorReportedState);
                }
            }
            CollectorsConfigurations = collectors;
        }
    }
}
