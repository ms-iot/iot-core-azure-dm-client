using System.IO;
using Microsoft.WindowsAzure.Storage;       // Namespace for CloudStorageAccount
using Microsoft.WindowsAzure.Storage.Blob;  // Namespace for Blob storage types

namespace DMDashboard
{
    class AzureStorageHelpers
    {
        public static void DownloadAzureFile(string connectionString, string containerName, string blobName, string targetFolder)
        {
            System.IO.Directory.CreateDirectory(targetFolder);

            CloudStorageAccount storageAccount = CloudStorageAccount.Parse(connectionString);
            CloudBlobClient blobClient = storageAccount.CreateCloudBlobClient();
            CloudBlobContainer container = blobClient.GetContainerReference(containerName);
            CloudBlockBlob blob = container.GetBlockBlobReference(blobName);
            blob.DownloadToFile(targetFolder + "\\" + blobName, FileMode.Create);
        }
    }
}
