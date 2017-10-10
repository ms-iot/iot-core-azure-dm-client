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
using System;
using System.Collections.ObjectModel;
using System.IO;
using System.Windows;
using System.Windows.Controls;

namespace DMDashboard
{
    /// <summary>
    /// Interaction logic for WifiReportedStateControl.xaml
    /// </summary>
    public partial class WifiReportedStateControl : UserControl
    {
        public WifiReportedStateControl()
        {
            InitializeComponent();
        }

        private static string GetBlobName(string profileName) => $"exported_{profileName}.xml";

        private void OnWifiProfileDetails_Upload(object sender, RoutedEventArgs e)
        {
            var parent = System.Windows.Media.VisualTreeHelper.GetParent(sender as DependencyObject);
            while (parent != null)
            {
                var mainWindow = parent as MainWindow;
                if (mainWindow != null)
                {
                    var wifiProfile = DataContext as WifiProfileConfiguration;
                    var profileName = wifiProfile.Name;
                    mainWindow.ExportWifiProfileDetails(
                        profileName,
                        mainWindow.AzureStorageDesiredConnectionString.Text,
                        mainWindow.AzureStorageContainerName.Text,
                        GetBlobName(profileName));
                    return;
                }
                parent = System.Windows.Media.VisualTreeHelper.GetParent(parent);
            }
        }
        private void OnWifiProfileDetails_View(object sender, RoutedEventArgs e)
        {
            var parent = System.Windows.Media.VisualTreeHelper.GetParent(sender as DependencyObject);
            while (parent != null)
            {
                var mainWindow = parent as MainWindow;
                if (mainWindow != null)
                {
                    var storageConnectionString = mainWindow.AzureStorageDesiredConnectionString.Text;
                    var storageContainerName = mainWindow.AzureStorageContainerName.Text;
                    var wifiProfile = DataContext as WifiProfileConfiguration;
                    string blobName = GetBlobName(wifiProfile.Name);

                    string wifiString = LoadFromAzureBlob(storageConnectionString, storageContainerName, blobName);

                    WifiDetails details = new WifiDetails();
                    details.Owner = mainWindow;
                    details.Xml = wifiString;
                    details.ShowDialog();

                    return;
                }
                parent = System.Windows.Media.VisualTreeHelper.GetParent(parent);
            }
        }

        private string LoadFromAzureBlob(string connectionString, string containerName, string blobName)
        {
            string tempPath = Path.GetTempPath();
            AzureStorageHelpers.DownloadAzureFile(connectionString, containerName, blobName, tempPath);
            string fullFileName = tempPath + blobName;
            if (!File.Exists(fullFileName))
            {
                throw new Exception("Error: failed to download wifi xml file!");
            }
            try
            {
                string wifiString = File.ReadAllText(fullFileName);
                return wifiString;
            }
            finally
            {
                File.Delete(fullFileName);
            }
        }
    }
}
