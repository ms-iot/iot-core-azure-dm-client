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
using System.Collections.ObjectModel;
using System.Text;
using System.Windows.Controls;

namespace DMDashboard.Wifi
{
    /// <summary>
    /// Interaction logic for WifiProfilesDesiredStateControl.xaml
    /// </summary>
    public partial class WifiProfilesDesiredStateControl : UserControl
    {
        public WifiProfilesDesiredStateControl()
        {
            InitializeComponent();
            this.desiredList.ItemsSource = DesiredList;
        }

        private ObservableCollection<WifiProfileConfiguration> _DesiredList = new ObservableCollection<WifiProfileConfiguration>();
        public ObservableCollection<WifiProfileConfiguration> DesiredList { get { return _DesiredList; } }

        public string ToJson()
        {
            StringBuilder sb = new StringBuilder("\"wifi\": ");

            if (DesiredList.Count == 0) sb.Append("\"no-no\"\n");
            else
            {
                bool first = true;
                sb.Append("{\n\"applyProperties\": {");
                foreach (var profile in DesiredList)
                {
                    if (!first)
                    {
                        sb.Append(",");
                    }
                    sb.Append("\n");
                    if (profile.Uninstall)
                    {
                        sb.Append($"\"{profile.Name}\": \"uninstall\"");
                    }
                    else
                    {
                        var disable = profile.DisableInternetConnectivityChecks ? "true" : "false";
                        sb.Append($"\"{profile.Name}\": {{\n\"profile\": \"{profile.Profile}\",\n\"disableInternetConnectivityChecks\": {disable}\n}}");
                    }
                    first = false;
                }
                sb.Append( "\n},\n");
                sb.Append("\"reportProperties\": \"yes\"\n}");
            }

            return sb.ToString();
        }
    }
}
