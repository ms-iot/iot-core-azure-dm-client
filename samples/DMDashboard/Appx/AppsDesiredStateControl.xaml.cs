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
using System.Windows.Controls;

namespace DMDashboard
{
    public partial class AppsDesiredStateControl : UserControl
    {
        public string SectionName
        {
            get
            {
                return "apps";
            }
        }

        public List<AppDesiredState> AppsConfigurations
        {
            set
            {
                ControlList.Children.Clear();
                for (int i = 0; i < value.Count; ++i)
                {
                    AppDesiredStateControl appDesiredStateControl = new AppDesiredStateControl();
                    appDesiredStateControl.DataContext = value[i];
                    ControlList.Children.Add(appDesiredStateControl);
                }
            }
        }

        public AppsDesiredStateControl()
        {
            InitializeComponent();
        }

        public string ToJson()
        {
            StringBuilder sb = new StringBuilder();

            string reportAllStoreAppsString = ReportAllStoreApps.IsChecked == true ? "true" : "false";
            string reportAllNonStoreAppsString = ReportAllNonStoreApps.IsChecked == true ? "true" : "false";

            sb.Append("\"?\": {\n");
            sb.Append("    \"store\": " + reportAllStoreAppsString + ",\n");
            sb.Append("    \"nonStore\": " + reportAllNonStoreAppsString + "\n");
            sb.Append("}\n");

            foreach (AppDesiredStateControl control in ControlList.Children)
            {
                string s = control.ToJson();
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
                return "\"apps\" : null";
            }

            return "\"apps\" : {\n" + sb.ToString() + "\n}";
        }

        private AppDesiredState AppDesiredStateFromJson(string pkgFamilyId, JToken jValueToken)
        {
            if (!(jValueToken is JObject))
            {
                return null;
            }
            JObject jValueObject = (JObject)jValueToken;

            string version = (string)jValueObject.GetValue("version");
            AppDesiredState.DesiredState desiredState = AppDesiredState.DesiredState.Reported;
            if (version == "?")
            {
                desiredState = AppDesiredState.DesiredState.Reported;
            }
            else if (version == "not installed")
            {
                desiredState = AppDesiredState.DesiredState.NotInstalled;
            }
            else
            {
                desiredState = AppDesiredState.DesiredState.Installed;
            }

            AppDesiredState.StartupState startupState = AppDesiredState.StartupState.None;
            string startUp = (string)jValueObject.GetValue("startUp");
            if (startUp == "none")
            {
                startupState = AppDesiredState.StartupState.None;
            }
            else if (startUp == "foreground")
            {
                startupState = AppDesiredState.StartupState.Foreground;
            }
            else if (startUp == "background")
            {
                startupState = AppDesiredState.StartupState.Background;
            }

            string dependencies = (string)jValueObject.GetValue("depsSources");
            string[] dependenciesList = dependencies.Split(';');

            AppDesiredState appDesiredState = new AppDesiredState(
                (string)jValueObject.GetValue("pkgFamilyName"),
                desiredState,
                version,
                startupState,
                (string)jValueObject.GetValue("appxSource"),
                dependenciesList[0],
                dependenciesList.Length > 1 ? dependenciesList[1] : "empty",
                (string)jValueObject.GetValue("certSource"),
                (string)jValueObject.GetValue("certStore"));

            return appDesiredState;
        }

        public void FromJson(JObject jRoot)
        {
            JToken jAppsToken = jRoot.SelectToken("properties.desired.windows.apps");
            if (jAppsToken == null || !(jAppsToken is JObject))
            {
                return;
            }
            JObject jAppsObject = (JObject)jAppsToken;

            List<AppDesiredState> apps = new List<AppDesiredState>();
            foreach (JProperty property in jAppsObject.Children())
            {
                AppDesiredState appDesiredState = AppDesiredStateFromJson(property.Name, property.Value);
                if (appDesiredState != null)
                {
                    apps.Add(appDesiredState);
                }
            }
            AppsConfigurations = apps;
        }

        private void OnAddAppConfiguration(object sender, System.Windows.RoutedEventArgs e)
        {
            ControlList.Children.Add(new AppDesiredStateControl());
        }
    }
}
