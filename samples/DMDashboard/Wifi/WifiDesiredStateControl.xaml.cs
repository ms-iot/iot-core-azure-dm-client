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
using System.Text;
using System.Windows.Controls;

namespace DMDashboard.Wifi
{
    /// <summary>
    /// Interaction logic for WifiDesiredStateControl.xaml
    /// </summary>
    public partial class WifiDesiredStateControl : UserControl
    {
        public WifiDesiredStateControl()
        {
            InitializeComponent();
        }

        public string ToJson()
        {
            var profileName = this.Name;
            StringBuilder sb = new StringBuilder($"\"{profileName}\": ");
            if (this.Uninstall.IsChecked.Value) sb.Append("uninstall");
            else
            {
                sb.Append("{");
                sb.Append($"\"profile\": \"{this.Profile}\", ");
                sb.Append($"\"disableInternetConnectivityChecks\": \"{this.DisableInternetConnectivityChecks.IsChecked.Value}\", ");
                sb.Append("}");
            };
            return sb.ToString();
        }
    }
}
