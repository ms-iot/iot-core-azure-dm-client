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

using Microsoft.WindowsAzure.Storage;       // Namespace for CloudStorageAccount
using Microsoft.WindowsAzure.Storage.Blob;  // Namespace for Blob storage types
using System;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;

namespace DMDashboard.StorageManagement
{
    public partial class DeviceUploadFile : Window
    {
        const string PropFolder = "folder";
        const string PropFile = "file";
        const string PropConnectionString = "connectionString";
        const string PropContainerName = "container";
        const string UploadDMFileMethod = "windows.uploadDMFile";

        public DeviceUploadFile(DeviceTwinAndMethod azureDevice)
        {
            InitializeComponent();

            _azureDevice = azureDevice;
            FileSelectionControl.AzureDevice = _azureDevice;
        }

        private void OnEnumContainers(object sender, RoutedEventArgs e)
        {
            CloudStorageAccount storageAccount = CloudStorageAccount.Parse(AzureStorageConnectionString.Text);
            CloudBlobClient blobClient = storageAccount.CreateCloudBlobClient();

            ContainersList.Items.Clear();
            foreach (var container in blobClient.ListContainers("", ContainerListingDetails.None, null, null))
            {
                ContainersList.Items.Add(container.Name);
            }
        }

        private async Task UploadAsync()
        {
            string fileName = FileSelectionControl.SelectedFileName;
            if (String.IsNullOrEmpty(fileName))
            {
                return;
            }

            string folderName = FileSelectionControl.SelectedFolderName;
            if (String.IsNullOrEmpty(folderName))
            {
                return;
            }

            if (String.IsNullOrEmpty(AzureStorageConnectionString.Text))
            {
                MessageBox.Show("Need to specify the Azure Storage connection string!");
                return;
            }
            string connectionString = AzureStorageConnectionString.Text;

            if (ContainersList.SelectedIndex == -1)
            {
                MessageBox.Show("Need to select a target container first!");
                return;
            }
            string containerName = (string)ContainersList.SelectedItem;


            CancellationToken cancellationToken = new CancellationToken();
            StringBuilder parameters = new StringBuilder();
            parameters.Append("{\n");
            parameters.Append("    \"" + PropFolder + "\": \"" + folderName + "\",");
            parameters.Append("    \"" + PropFile + "\": \"" + fileName + "\",");
            parameters.Append("    \"" + PropConnectionString + "\": \"" + connectionString + "\",");
            parameters.Append("    \"" + PropContainerName + "\": \"" + containerName + "\"");
            parameters.Append("}\n");
            DeviceMethodReturnValue result = await _azureDevice.CallDeviceMethod(UploadDMFileMethod, parameters.ToString(), new TimeSpan(0, 0, 30), cancellationToken);

            MessageBox.Show("Triggered file upload...");
        }

        private void OnUpload(object sender, RoutedEventArgs e)
        {
            UploadAsync().FireAndForget();
        }

        private void OnClose(object sender, RoutedEventArgs e)
        {
            Close();
        }

        DeviceTwinAndMethod _azureDevice;
    }
}
