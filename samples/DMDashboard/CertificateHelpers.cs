using System;
using System.IO;
using System.Security.Cryptography.X509Certificates;

namespace DMDashboard
{
    class CertificateHelpers
    {
        public static X509Certificate2 GetCertificateInfo(string connectionString, string containerName, string blobName, string targetFolder)
        {
            AzureStorageHelpers.DownloadAzureFile(connectionString, containerName, blobName, targetFolder);

            string fullFileName = targetFolder + "\\" + blobName;
            if (!File.Exists(fullFileName))
            {
                throw new Exception("Error: failed to download certificate file!");
            }

            X509Certificate certp = X509Certificate2.CreateFromSignedFile(fullFileName);
            return new X509Certificate2(certp.Handle);
        }
    }
}
