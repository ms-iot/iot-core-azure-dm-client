using System;
using Newtonsoft.Json.Linq;
using System.Diagnostics;
using Microsoft.Devices.Management;
using System.Collections.Generic;
using System.Threading.Tasks;

namespace IoTDMClient
{
    class CertificateManagement
    {
        public static Microsoft.Devices.Management.Message.CertificateConfiguration GetDesiredCertificateConfiguration(JProperty certificateConfigurationJson)
        {
            if (certificateConfigurationJson.Value.Type != JTokenType.Object)
            {
                // report incorrect json.
                return null;
            }

            Debug.WriteLine("certificates = " + certificateConfigurationJson.Value.ToString());
            JObject subProperties = (JObject)certificateConfigurationJson.Value;

            Microsoft.Devices.Management.Message.CertificateConfiguration certificateConfiguration = new Microsoft.Devices.Management.Message.CertificateConfiguration();
            certificateConfiguration.rootCATrustedCertificates_Root = (string)subProperties.Property("rootCATrustedCertificates_Root").Value;
            certificateConfiguration.rootCATrustedCertificates_CA = (string)subProperties.Property("rootCATrustedCertificates_CA").Value;
            certificateConfiguration.rootCATrustedCertificates_TrustedPublisher = (string)subProperties.Property("rootCATrustedCertificates_TrustedPublisher").Value;
            certificateConfiguration.rootCATrustedCertificates_TrustedPeople = (string)subProperties.Property("rootCATrustedCertificates_TrustedPeople").Value;
            certificateConfiguration.certificateStore_CA_System = (string)subProperties.Property("certificateStore_CA_System").Value;
            certificateConfiguration.certificateStore_Root_System = (string)subProperties.Property("certificateStore_Root_System").Value;
            certificateConfiguration.certificateStore_My_User = (string)subProperties.Property("certificateStore_My_User").Value;
            certificateConfiguration.certificateStore_My_System = (string)subProperties.Property("certificateStore_My_System").Value;

            return certificateConfiguration;
        }

        private static async Task DownloadCertificates(DeviceManagementClient client, string connectionString, string containerName, HashSet<string> certificateFilesSet)
        {
            // ToDo: since our cache is temporary, we might have to download those files everytime to verify the hashes.
            if (certificateFilesSet == null || certificateFilesSet.Count == 0)
            {
                return;
            }
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
            foreach (string hash in hashes)
            {
                certificateFilesSet.Add(hash);
            }
        }

        public static async Task DownloadCertificates(
            DeviceManagementClient client,
            string connectionString,
            string containerName,
            Microsoft.Devices.Management.Message.CertificateConfiguration certificateConfiguration)
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
