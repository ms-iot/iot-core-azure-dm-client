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
using System.Windows;
using System.Windows.Controls;

namespace DMDashboard
{
    public partial class AppsReportedStateControl : UserControl
    {
        public AppReportedState[] AppsStatusData
        {
            set
            {
                AppReportedState[] states = (AppReportedState[])value;
                AppReportedStateControl[] controls = { App0, App1, App2, App3 };
                for (int i = 0; i < states.Length; ++i)
                {
                    controls[i].DataContext = states[i];
                }
                for (int i = states.Length; i < 4; ++i)
                {
                    controls[i].DataContext = new AppReportedState();
                }
            }
        }

        public AppsReportedStateControl()
        {
            InitializeComponent();
        }

        public void AppsStatusJsonToUI(JToken token)
        {
            if (!(token is JObject))
            {
                MessageBox.Show("Error: invalid apps node json format!");
                return;
            }

            List<AppReportedState> data = new List<AppReportedState>();

            JObject root = (JObject)token;
            foreach (JToken p in root.Children())
            {
                if (!(p is JProperty))
                {
                    continue;
                }
                JProperty property = (JProperty)p;
                string packageFamilyJsonId = property.Name;
                if (property.Value == null)
                {
                    data.Add(new AppReportedState(packageFamilyJsonId, null, null, null));
                }
                else if (property.Value is JObject)
                {
                    AppReportedState appStatusData = new AppReportedState();
                    JObject packageFamilyProperties = (JObject)property.Value;
                    foreach (JToken p1 in packageFamilyProperties.Children())
                    {
                        if (!(p1 is JProperty))
                        {
                            continue;
                        }
                        JProperty property1 = (JProperty)p1;
                        if (property1.Name == "pkgFamilyName")
                        {
                            appStatusData.PackageFamilyName = property1.Value.ToString();
                        }
                        else if (property1.Name == "version")
                        {
                            appStatusData.Version = property1.Value.ToString();
                        }
                        else if (property1.Name == "installDate")
                        {
                            appStatusData.InstallDate = property1.Value.ToString();
                        }
                        else if (property1.Name == "error")
                        {
                            appStatusData.Error = property1.Value.ToString();
                        }
                    }
                    data.Add(appStatusData);
                }
            }
            AppsStatusData = data.ToArray();
        }
    }
}
