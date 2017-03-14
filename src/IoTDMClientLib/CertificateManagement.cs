using System;
using Newtonsoft.Json.Linq;
using System.Diagnostics;
using Microsoft.Devices.Management;
using Microsoft.Devices.Management.Message;
using System.Collections.Generic;
using System.Threading.Tasks;

namespace IoTDMClient
{
    class CertificateManagement
    {
        private static async Task DownloadCertificates(DeviceManagementClient client, string connectionString, string containerName, HashSet<string> certificateFilesSet)
        {
            // ToDo: since our cache is temporary, we might have to download those files everytime to verify the hashes.
            Debug.Assert(certificateFilesSet != null);

            foreach (string fileName in certificateFilesSet)
            {
                IoTDMClient.BlobInfo blobInfo = new IoTDMClient.BlobInfo();
                blobInfo.ConnectionString = connectionString;
                blobInfo.ContainerName = containerName;
                blobInfo.BlobName = fileName;
                Debug.WriteLine("Downloading " + blobInfo.BlobName);
                await blobInfo.DownloadToTempAsync(client);
            }
        }

        private static void MergeCertificateFileNames(string hashesString, HashSet<string> certificateFilesSet)
        {
            char separator = '/';

            if (String.IsNullOrEmpty(hashesString))
            {
                return;
            }
            string[] hashes = hashesString.Split(separator);
            certificateFilesSet.UnionWith(hashes);
        }

        public static async Task DownloadCertificates(
            DeviceManagementClient client,
            string connectionString,
            string containerName,
            CertificateConfiguration certificateConfiguration)
        {
            HashSet<string> certificateFilesSet = new HashSet<string>();

            MergeCertificateFileNames(certificateConfiguration.rootCATrustedCertificates_Root, certificateFilesSet);
            MergeCertificateFileNames(certificateConfiguration.rootCATrustedCertificates_CA, certificateFilesSet);
            MergeCertificateFileNames(certificateConfiguration.rootCATrustedCertificates_TrustedPublisher, certificateFilesSet);
            MergeCertificateFileNames(certificateConfiguration.rootCATrustedCertificates_TrustedPeople, certificateFilesSet);

            MergeCertificateFileNames(certificateConfiguration.certificateStore_CA_System, certificateFilesSet);
            MergeCertificateFileNames(certificateConfiguration.certificateStore_Root_System, certificateFilesSet);
            MergeCertificateFileNames(certificateConfiguration.certificateStore_My_User, certificateFilesSet);
            MergeCertificateFileNames(certificateConfiguration.certificateStore_My_System, certificateFilesSet);

            await DownloadCertificates(client, connectionString, containerName, certificateFilesSet);
        }

    }
}
