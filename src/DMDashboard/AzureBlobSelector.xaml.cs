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
