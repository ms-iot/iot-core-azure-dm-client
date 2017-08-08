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
using System.Collections.Generic;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Forms;

namespace DMDashboard.StorageManagement
{
    /// <summary>
    /// Interaction logic for AzureStorageManager.xaml
    /// </summary>
    public partial class AzureStorageManager : System.Windows.Controls.UserControl
    {
        class BlobInfo
        {
            public string Name { get; set; }
            public string Type { get; set; }
            public string Uri { get; set; }

            public BlobInfo(string name, string type, string uri)
            {
                this.Name = name;
                this.Type = type;
                this.Uri = uri;
            }
        }

        public string ConnectionString
        {
            get
            {
                return StorageConnectionStringBox.Text;
            }
            set
            {
                StorageConnectionStringBox.Text = value;
            }
        }


        public AzureStorageManager()
        {
            InitializeComponent();
        }

        private void ListContainers(string connectionString)
        {
            CloudStorageAccount storageAccount = CloudStorageAccount.Parse(connectionString);
            CloudBlobClient blobClient = storageAccount.CreateCloudBlobClient();

            ContainersList.Items.Clear();
            foreach (var container in blobClient.ListContainers("", ContainerListingDetails.None, null, null))
            {
                ContainersList.Items.Add(container.Name);
            }
        }

        private void OnListContainers(object sender, RoutedEventArgs e)
        {
            ListContainers(StorageConnectionStringBox.Text);
        }

        private void OnListBlobs(object sender, RoutedEventArgs e)
        {
            if (ContainersList.SelectedIndex == -1)
            {
                return;
            }

            CloudStorageAccount storageAccount = CloudStorageAccount.Parse(StorageConnectionStringBox.Text);
            CloudBlobClient blobClient = storageAccount.CreateCloudBlobClient();
            CloudBlobContainer container = blobClient.GetContainerReference((string)ContainersList.SelectedItem);

            List<BlobInfo> blobInfoList = new List<BlobInfo>();
            foreach (IListBlobItem item in container.ListBlobs(null, false))
            {
                if (item.GetType() == typeof(CloudBlockBlob))
                {
                    CloudBlockBlob blob = (CloudBlockBlob)item;
                    blobInfoList.Add(new BlobInfo(blob.Name, "BlockBlob", blob.Uri.ToString()));
                }
                else if (item.GetType() == typeof(CloudPageBlob))
                {
                    CloudPageBlob pageBlob = (CloudPageBlob)item;
                    blobInfoList.Add(new BlobInfo(pageBlob.Name, "PageBlob", pageBlob.Uri.ToString()));

                }
                else if (item.GetType() == typeof(CloudBlobDirectory))
                {
                    CloudBlobDirectory directoryBlob = (CloudBlobDirectory)item;
                    blobInfoList.Add(new BlobInfo("<dir>", "BlobDirectory", directoryBlob.Uri.ToString()));
                }
            }
            BlobsList.ItemsSource = blobInfoList;
        }

        private CloudBlockBlob GetBlobReference(string connectionString, string containerName, string blobName)
        {
            try
            {
                CloudStorageAccount storageAccount = CloudStorageAccount.Parse(connectionString);
                CloudBlobClient blobClient = storageAccount.CreateCloudBlobClient();
                CloudBlobContainer container = blobClient.GetContainerReference(containerName);
                return container.GetBlockBlobReference(blobName);
            }
            catch (Exception e)
            {
                System.Windows.MessageBox.Show("Error getting a blob reference for " + blobName + ". Error: " + e.Message);
                return null;
            }
        }

        private async Task DownloadAsync(string connectionString, string containerName, string blobName, string localFolder)
        {
            try
            {
                var blockBlob = GetBlobReference(connectionString, containerName, blobName);
                await blockBlob.DownloadToFileAsync(localFolder + "\\" + blobName, System.IO.FileMode.CreateNew);
                System.Windows.MessageBox.Show("Download complete!");
            }
            catch (Exception e)
            {
                System.Windows.MessageBox.Show("Error downloading the file! " + e.Message);
                return;
            }
        }

        private async Task DeleteAsync(string connectionString, string containerName, string blobName)
        {
            try
            {
                var blockBlob = GetBlobReference(connectionString, containerName, blobName);
                await blockBlob.DeleteAsync();
                System.Windows.MessageBox.Show("Delete complete!");

                OnListBlobs(null, null);
            }
            catch (Exception e)
            {
                System.Windows.MessageBox.Show("Error delete the file! " + e.Message);
                return;
            }
        }

        private void OnDownloadSelectedBlob(object sender, RoutedEventArgs e)
        {
            if (BlobsList.SelectedIndex == -1)
            {
                return;
            }

            FolderBrowserDialog folderBrowserDialog1 = new FolderBrowserDialog();
            DialogResult result = folderBrowserDialog1.ShowDialog();
            string localFolder = folderBrowserDialog1.SelectedPath;

            DownloadAsync(StorageConnectionStringBox.Text, (string)ContainersList.SelectedItem, ((BlobInfo)BlobsList.SelectedItem).Name, localFolder).FireAndForget();
        }

        private void OnDeleteSelectedBlob(object sender, RoutedEventArgs e)
        {
            if (BlobsList.SelectedIndex == -1)
            {
                return;
            }

            DeleteAsync(StorageConnectionStringBox.Text, (string)ContainersList.SelectedItem, ((BlobInfo)BlobsList.SelectedItem).Name).FireAndForget();
        }
    }
}
