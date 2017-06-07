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
using Newtonsoft.Json.Linq;
using System.Collections.ObjectModel;
using System.Windows;
using System.Windows.Controls;

namespace DMDashboard.Wifi
{
    /// <summary>
    /// Interaction logic for WifiProfilesReportedStateControl.xaml
    /// </summary>
    public partial class WifiProfilesReportedStateControl : UserControl
    {
        public WifiProfilesReportedStateControl()
        {
            InitializeComponent();
            this.reportedList.ItemsSource = ReportedList;
        }

        private ObservableCollection<WifiProfileConfiguration> _ReportedList = new ObservableCollection<WifiProfileConfiguration>();
        public ObservableCollection<WifiProfileConfiguration> ReportedList { get { return _ReportedList; } }

        public void FromJson(JToken token)
        {
            if (!(token is JObject))
            {
                MessageBox.Show("Error: invalid apps node json format!");
                return;
            }
            ReportedList.Clear();

            JObject root = (JObject)token;
            foreach (JToken p in root.Children())
            {
                if (!(p is JProperty))
                {
                    continue;
                }
                JProperty property = (JProperty)p;
                string profileName = property.Name;
                ReportedList.Add(new WifiProfileConfiguration() { Name = profileName });
            }
        }
    }
}
