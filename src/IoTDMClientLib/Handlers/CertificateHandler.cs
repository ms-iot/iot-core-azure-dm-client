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

using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using Microsoft.Devices.Management.DMDataContract;
using System;
using System.Diagnostics;
using System.Collections.Generic;
using System.IO;
using System.Text;
using System.Threading.Tasks;
using Windows.Foundation.Diagnostics;
using Windows.Storage;

namespace Microsoft.Devices.Management
{
    class CertificateHandler : IClientPropertyHandler, IClientPropertyDependencyHandler
    {
        string[] JsonSectionDependencyNames = { "externalStorage" };

        // IClientPropertyHandler
        public string PropertySectionName
        {
            get
            {
                return CertificatesDataContract.SectionName;
            }
        }

        public struct GetCertificateDetailsParams
        {
            public string path;
            public string hash;
            public string connectionString;
            public string containerName;
            public string blobName;
        }

        public CertificateHandler(
            IClientHandlerCallBack deviceManagementClient,
            ISystemConfiguratorProxy systemConfiguratorProxy)
        {
            _systemConfiguratorProxy = systemConfiguratorProxy;
            _deviceManagementClient = deviceManagementClient;
        }

        private async void GetCertificateDetailsAsync(string jsonParam)
        {
            GetCertificateDetailsParams parameters = JsonConvert.DeserializeObject<GetCertificateDetailsParams>(jsonParam);

            var request = new Message.GetCertificateDetailsRequest();
            request.path = parameters.path;
            request.hash = parameters.hash;

            Message.GetCertificateDetailsResponse response = await _systemConfiguratorProxy.SendCommandAsync(request) as Message.GetCertificateDetailsResponse;

            string jsonString = JsonConvert.SerializeObject(response);
            Debug.WriteLine("response = " + jsonString);

            var info = new Message.AzureFileTransferInfo()
            {
                ConnectionString = parameters.connectionString,
                ContainerName = parameters.containerName,
                BlobName = parameters.blobName,
                Upload = true,
                RelativeLocalPath = ""
            };

            var appLocalDataFile = await ApplicationData.Current.TemporaryFolder.CreateFileAsync(parameters.blobName, CreationCollisionOption.ReplaceExisting);
            using (StreamWriter writer = new StreamWriter(await appLocalDataFile.OpenStreamForWriteAsync()))
            {
                await writer.WriteAsync(jsonString);
            }
            await IoTDMClient.AzureBlobFileTransfer.UploadFile(info, appLocalDataFile);

            await appLocalDataFile.DeleteAsync();
        }

        public Task<string> GetCertificateDetailsHandlerAsync(string jsonParam)
        {
            Debug.WriteLine("GetCertificateDetailsHandlerAsync");

            var response = new { response = "succeeded", reason = "" };
            try
            {
                // Submit the work and return immediately.
                GetCertificateDetailsAsync(jsonParam);
            }
            catch (Exception e)
            {
                response = new { response = "rejected:", reason = e.Message };
            }

            return Task.FromResult(JsonConvert.SerializeObject(response));
        }

        private static string DesiredToString(List<CertificatesDataContract.CertificateInfo> certificates)
        {
            StringBuilder sb = new StringBuilder();
            bool first = true;
            foreach (CertificatesDataContract.CertificateInfo certificate in certificates)
            {
                if (!first)
                {
                    sb.Append(",");
                }
                if (certificate.State == CertificatesDataContract.JsonStateInstalled)
                {
                    sb.Append(CertificatesDataContract.JsonStateInstalled);
                    sb.Append("|");
                    sb.Append(certificate.StorageFileName);
                }
                else if (certificate.State == CertificatesDataContract.JsonStateUninstalled)
                {
                    sb.Append(CertificatesDataContract.JsonStateUninstalled);
                    sb.Append("|");
                    sb.Append(certificate.Hash);
                }
                else
                {
                    sb.Append(CertificatesDataContract.JsonStateUnknown);
                    sb.Append("|?");
                }

                first = false;
            }
            return sb.ToString();
        }

        // IClientPropertyHandler
        public async Task<CommandStatus> OnDesiredPropertyChange(JToken desiredValue)
        {
            if (!(desiredValue is JObject))
            {
                throw new Error(ErrorCodes.INVALID_DESIRED_JSON_VALUE, "Invalid json value type for the " + PropertySectionName + " node.");
            }

            CertificatesDataContract.DesiredProperties desiredProperties = CertificatesDataContract.DesiredProperties.FromJsonObject((JObject)desiredValue);

            await IoTDMClient.CertificateManagement.DownloadCertificates(_systemConfiguratorProxy, _connectionString, desiredProperties);

            Message.CertificateConfiguration certificateConfiguration = new Message.CertificateConfiguration();
            certificateConfiguration.certificateStore_CA_System = DesiredToString(desiredProperties.certificateStore_CA_System);
            certificateConfiguration.certificateStore_My_System = DesiredToString(desiredProperties.certificateStore_My_System);
            certificateConfiguration.certificateStore_My_User = DesiredToString(desiredProperties.certificateStore_My_User);
            certificateConfiguration.certificateStore_Root_System = DesiredToString(desiredProperties.certificateStore_Root_System);
            certificateConfiguration.rootCATrustedCertificates_CA = DesiredToString(desiredProperties.rootCATrustedCertificates_CA);
            certificateConfiguration.rootCATrustedCertificates_Root = DesiredToString(desiredProperties.rootCATrustedCertificates_Root);
            certificateConfiguration.rootCATrustedCertificates_TrustedPeople = DesiredToString(desiredProperties.rootCATrustedCertificates_TrustedPeople);
            certificateConfiguration.rootCATrustedCertificates_TrustedPublisher = DesiredToString(desiredProperties.rootCATrustedCertificates_TrustedPublisher);

            var request = new Message.SetCertificateConfigurationRequest(certificateConfiguration);
            await _systemConfiguratorProxy.SendCommandAsync(request);

            JObject reportedProperties = await GetReportedPropertyAsync();

            Debug.WriteLine("-- Reporting Certificates -------------------------------------");
            Debug.WriteLine(reportedProperties.ToString());
            Debug.WriteLine("-- Reporting Certificates Done --------------------------------");

            // Because the section contains a list, we need to reset the parent to allow for removals...
            await _deviceManagementClient.ReportPropertiesAsync(PropertySectionName, new JValue("refreshing"));

            // Report the updated list...
            await _deviceManagementClient.ReportPropertiesAsync(PropertySectionName, reportedProperties);

            return CommandStatus.Committed;
        }

        // IClientPropertyHandler
        public async Task<JObject> GetReportedPropertyAsync()
        {
            var request = new Message.GetCertificateConfigurationRequest();
            var response = await _systemConfiguratorProxy.SendCommandAsync(request) as Message.GetCertificateConfigurationResponse;

            CertificatesDataContract.ReportedProperties reportedProperties = new CertificatesDataContract.ReportedProperties();

            reportedProperties.Set(CertificatesDataContract.JsonRootCATrustedCertificates_Root, response.configuration.rootCATrustedCertificates_Root);
            reportedProperties.Set(CertificatesDataContract.JsonRootCATrustedCertificates_CA, response.configuration.rootCATrustedCertificates_CA);
            reportedProperties.Set(CertificatesDataContract.JsonRootCATrustedCertificates_TrustedPublisher, response.configuration.rootCATrustedCertificates_TrustedPublisher);
            reportedProperties.Set(CertificatesDataContract.JsonRootCATrustedCertificates_TrustedPeople, response.configuration.rootCATrustedCertificates_TrustedPeople);
            reportedProperties.Set(CertificatesDataContract.JsonCertificateStore_CA_System, response.configuration.certificateStore_CA_System);
            reportedProperties.Set(CertificatesDataContract.JsonCertificateStore_Root_System, response.configuration.certificateStore_Root_System);
            reportedProperties.Set(CertificatesDataContract.JsonCertificateStore_My_User, response.configuration.certificateStore_My_User);
            reportedProperties.Set(CertificatesDataContract.JsonCertificateStore_My_System, response.configuration.certificateStore_My_System);

            return reportedProperties.ToJsonObject();
        }

        // IClientPropertyDependencyHandler
        public string[] PropertySectionDependencyNames
        {
            get
            {
                return JsonSectionDependencyNames; // todo: constant in data contract?
            }
        }

        // IClientPropertyDependencyHandler
        public void OnDesiredPropertyDependencyChange(string section, JObject value)
        {
            if (section.Equals(JsonSectionDependencyNames[0]))
            {
                // externalStorage
                this._connectionString = (string)value.Property("connectionString").Value;
            }
        }

        private string _connectionString;
        ISystemConfiguratorProxy _systemConfiguratorProxy;
        private IClientHandlerCallBack _deviceManagementClient;
    }
}