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
using Newtonsoft.Json.Linq;
using Microsoft.Devices.Management;
using Microsoft.Devices.Management.DMDataContract;
using Microsoft.Devices.Management.Message;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading.Tasks;

namespace IoTDMClient
{
    class CertificateManagement
    {
        private static async Task DownloadCertificates(ISystemConfiguratorProxy systemConfiguratorProxy, string connectionString, HashSet<string> certificateFilesSet)
        {
            // ToDo: since our cache is temporary, we might have to download those files everytime to verify the hashes.
            Debug.Assert(certificateFilesSet != null);

            foreach (string fileName in certificateFilesSet)
            {
                string [] fileNameParts = fileName.Split('\\');
                if (fileNameParts.Length != 2)
                {
                    throw new Exception("Incorrect azure storage specification! The format should be containerName\\blobName.");
                }
                IoTDMClient.BlobInfo blobInfo = new IoTDMClient.BlobInfo();
                blobInfo.ConnectionString = connectionString;
                blobInfo.ContainerName = fileNameParts[0];
                blobInfo.BlobName = fileNameParts[1];
                Debug.WriteLine("Downloading " + blobInfo.BlobName);
                await blobInfo.DownloadToTempAsync(systemConfiguratorProxy);
            }
        }

        private static void MergeCertificateFileNames(List<CertificatesDataContract.CertificateInfo> certificates, HashSet<string> certificateFilesSet)
        {
            foreach (CertificatesDataContract.CertificateInfo certificate in certificates)
            {
                if (certificate.State == CertificatesDataContract.JsonStateInstalled)
                {
                    certificateFilesSet.Add(certificate.StorageFileName);
                }
            }
        }

        public static async Task DownloadCertificates(
            ISystemConfiguratorProxy systemConfiguratorProxy,
            string connectionString,
            CertificatesDataContract.DesiredProperties desiredProperties)
        {
            HashSet<string> certificateFilesSet = new HashSet<string>();

            MergeCertificateFileNames(desiredProperties.rootCATrustedCertificates_Root, certificateFilesSet);
            MergeCertificateFileNames(desiredProperties.rootCATrustedCertificates_CA, certificateFilesSet);
            MergeCertificateFileNames(desiredProperties.rootCATrustedCertificates_TrustedPublisher, certificateFilesSet);
            MergeCertificateFileNames(desiredProperties.rootCATrustedCertificates_TrustedPeople, certificateFilesSet);

            MergeCertificateFileNames(desiredProperties.certificateStore_CA_System, certificateFilesSet);
            MergeCertificateFileNames(desiredProperties.certificateStore_Root_System, certificateFilesSet);
            MergeCertificateFileNames(desiredProperties.certificateStore_My_User, certificateFilesSet);
            MergeCertificateFileNames(desiredProperties.certificateStore_My_System, certificateFilesSet);

            await DownloadCertificates(systemConfiguratorProxy, connectionString, certificateFilesSet);
        }

    }
}
