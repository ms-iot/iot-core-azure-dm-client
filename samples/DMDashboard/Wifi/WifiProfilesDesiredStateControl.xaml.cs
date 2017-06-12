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
using System.Windows;
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
            DesiredList = new ObservableCollection<WifiProfileConfiguration>();
            this.desiredList.ItemsSource = DesiredList;

            DesiredList.CollectionChanged += (source, e) => {
                itemsPanel.Visibility = (DesiredList.Count > 0) ? Visibility.Visible : Visibility.Collapsed;
                emptyIndicator.Visibility = (DesiredList.Count == 0) ? Visibility.Visible : Visibility.Collapsed;
            };
        }

        private ObservableCollection<WifiProfileConfiguration> DesiredList { get; set; }

        public void AddProfile(WifiProfileConfiguration newProfile)
        {
            DesiredList.Add(newProfile);
        }

        private void OnAddProfile(object sender, RoutedEventArgs e)
        {
            AddProfile(new Wifi.WifiProfileConfiguration());
        }

        public string ToJson()
        {
            return WifiProfileConfiguration.ToJsonString(DesiredList, this.ApplyPropertiesCombo.PropertyValue, this.reportPropertiesCombo.PropertyValue);
        }
    }
}
