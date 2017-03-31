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
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Windows;
using Microsoft.WindowsAzure.Storage;
using Microsoft.WindowsAzure.Storage.Blob;

namespace DMDashboard
{
    public partial class AzureBlobSelector : Window
    {
        public class BlobData
        {
            public bool IsSelected { get; set; }
            public string FileName { get; set; }

            public BlobData(bool isSelected, string fileName)
            {
                this.IsSelected = isSelected;
                this.FileName = fileName;
            }
        }

        public List<string> BlobFileNames { get; private set; }

        public AzureBlobSelector(IEnumerable<string> fileNames, string connectionString, string containerName, string extensionFilter)
        {
            InitializeComponent();
            PopulateBlobList(fileNames, connectionString, containerName, extensionFilter);
        }

        private void PopulateBlobList(IEnumerable<string> fileNames, string connectionString, string containerName, string extensionFilter)
        {
            CloudStorageAccount storageAccount = CloudStorageAccount.Parse(connectionString);
            CloudBlobClient blobClient = storageAccount.CreateCloudBlobClient();
            CloudBlobContainer container = blobClient.GetContainerReference(containerName);

            _blobInfoList = new List<BlobData>();
            foreach (IListBlobItem item in container.ListBlobs(null, false))
            {
                if (item.GetType() == typeof(CloudBlockBlob))
                {
                    CloudBlockBlob blob = (CloudBlockBlob)item;
                    string extension = Path.GetExtension(blob.Name);
                    if (string.IsNullOrEmpty(extensionFilter) || extension.Equals(extensionFilter, System.StringComparison.OrdinalIgnoreCase))
                    {
                        bool selected = fileNames.Any(x => x == blob.Name);
                        _blobInfoList.Add(new BlobData(selected, blob.Name));
                    }
                }
            }
            BlobList.ItemsSource = _blobInfoList;
        }

        private void OnApply(object sender, RoutedEventArgs e)
        {
            Close();
            BlobFileNames = new List<string>();
            foreach (BlobData blob in _blobInfoList)
            {
                if (blob.IsSelected)
                {
                    BlobFileNames.Add(blob.FileName);
                }
            }
        }

        List<BlobData> _blobInfoList;
    }
}
