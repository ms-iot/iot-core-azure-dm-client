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
using IoTDMClient;
using Microsoft.Devices.Management.Message;
using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using Windows.Data.Xml.Dom;
using Windows.Storage;

namespace Microsoft.Devices.Management
{
    class WifiManagement
    {
        private static IoTDMClient.BlobInfo BlobInfoFromSource(string connectionString, string source)
        {
            string[] sourceParts = source.Split('/');
            if (sourceParts.Length != 2)
            {
                throw new Exception("container name is missing in: " + source);
            }
            IoTDMClient.BlobInfo info = new IoTDMClient.BlobInfo();
            info.ConnectionString = connectionString;
            info.ContainerName = sourceParts[0];
            info.BlobName = sourceParts[1];
            return info;
        }

        public static async Task UpdateConfigWithProfileXmlAsync(string connectionString, IEnumerable<Message.WifiProfileConfiguration> profilesToAdd)
        {
            // Download missing profiles
            foreach (var profile in profilesToAdd)
            {
                var profileBlob = BlobInfoFromSource(connectionString, profile.Path);
                AzureFileTransferInfo info = new AzureFileTransferInfo() { BlobName=profileBlob.BlobName, ConnectionString = profileBlob.ConnectionString, ContainerName = profileBlob.ContainerName };
                var storageFile = await ApplicationData.Current.TemporaryFolder.CreateFileAsync(profileBlob.BlobName, CreationCollisionOption.ReplaceExisting);
                var localProfilePath = await AzureBlobFileTransfer.DownloadFile(info, storageFile);

                // strip off temp folder prefix for use with TemporaryFolder.CreateFileAsync
                var contents = await Windows.Storage.FileIO.ReadTextAsync(storageFile);
                var encodedXml = new System.Xml.Linq.XElement("Data", contents);
                profile.Xml = encodedXml.FirstNode.ToString();
                //profile.Xml = SecurityElement.Escape(contents);
                await storageFile.DeleteAsync();
            }
        }
    }
}
