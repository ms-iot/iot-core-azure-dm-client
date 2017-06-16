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
        private static async Task DownloadCertificates(ISystemConfiguratorProxy systemConfiguratorProxy, string connectionString, string containerName, HashSet<string> certificateFilesSet)
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
                await blobInfo.DownloadToTempAsync(systemConfiguratorProxy);
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
            ISystemConfiguratorProxy systemConfiguratorProxy,
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

            await DownloadCertificates(systemConfiguratorProxy, connectionString, containerName, certificateFilesSet);
        }

    }
}
