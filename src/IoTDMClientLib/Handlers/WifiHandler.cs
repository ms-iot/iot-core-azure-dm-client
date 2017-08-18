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

using Microsoft.Devices.Management.DMDataContract;
using IoTDMClient;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using Microsoft.Devices.Management.Message;
using System;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using System.Diagnostics;
using System.Collections.Generic;
using Windows.Storage;

namespace Microsoft.Devices.Management
{
    class WifiHandler : IClientPropertyHandler, IClientPropertyDependencyHandler, IClientDirectMethodHandler
    {
        const string JsonSectionName = "wifi";
        string [] JsonSectionDependencyNames = { "externalStorage" };

        public WifiHandler(IClientHandlerCallBack callback, ISystemConfiguratorProxy systemConfiguratorProxy)
        {
            this._systemConfiguratorProxy = systemConfiguratorProxy;
            this._callback = callback;
        }

        // IClientDirectMethodHandler
        public IReadOnlyDictionary<string, Func<string, Task<string>>> GetDirectMethodHandler()
        {
            return new Dictionary<string, Func<string, Task<string>>>()
                {
                    { DMJSonConstants.DTWindowsIoTNameSpace + ".getWifiDetails" , GetWifiProfileDetailsAsync },
                };
        }

        // IClientPropertyHandler
        public string PropertySectionName
        {
            get
            {
                return JsonSectionName; // todo: constant in data contract?
            }
        }

        private class WifiProfileConfigurationNameComparer : IEqualityComparer<WifiProfileConfiguration>
        {
            public bool Equals(WifiProfileConfiguration x, WifiProfileConfiguration y)
            {
                return (x.Name == y.Name);
            }
            public int GetHashCode(WifiProfileConfiguration x)
            {
                return x.Name.GetHashCode();
            }

        }

        private async Task<Message.GetWifiConfigurationResponse> GetWifiConfigurationAsync()
        {
            var request = new Message.GetWifiConfigurationRequest();
            var response = (await this._systemConfiguratorProxy.SendCommandAsync(request) as Message.GetWifiConfigurationResponse);

            if (Debugger.IsAttached)
            {
                foreach (var profile in response.Configuration.Profiles)
                {
                    Debug.WriteLine($"{profile.Name} : path={profile.Path} uninstall={profile.Uninstall}");
                }
            }

            return response;
        }

        public async Task UpdateConfigWithProfileXmlAsync(string connectionString, IEnumerable<Message.WifiProfileConfiguration> profilesToAdd)
        {
            // Download missing profiles
            foreach (var profile in profilesToAdd)
            {
                var profileBlob = IoTDMClient.BlobInfo.BlobInfoFromSource(connectionString, profile.Path);
                AzureFileTransferInfo info = new AzureFileTransferInfo() { BlobName = profileBlob.BlobName, ConnectionString = profileBlob.ConnectionString, ContainerName = profileBlob.ContainerName };
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

        private async Task ProcessDesiredWifiConfigurationAsync(
            string connectionString,
            Message.WifiConfiguration desiredConfiguration)
        {
            bool desireReported = desiredConfiguration.ReportToDeviceTwin == DMJSonConstants.YesString;
            IEnumerable<WifiProfileConfiguration> needToRemove;
            if (desiredConfiguration.ApplyFromDeviceTwin == DMJSonConstants.YesString)
            {
                // Get installed wifi profiles
                var getInstalledResponse = await GetWifiConfigurationAsync();
                var reportedConfigurationProfiles = getInstalledResponse.Configuration.Profiles;
                var desiredConfigurationProfiles = desiredConfiguration.Profiles;

                // Only profiles that don't already exist need to be installed
                var needToAdd = desiredConfigurationProfiles.Where((config) => { return !config.Uninstall; }).Except(reportedConfigurationProfiles, new WifiProfileConfigurationNameComparer());
                // Only profiles that already exist need to be uninstalled
                needToRemove = desiredConfigurationProfiles.Where((config) => { return config.Uninstall; }).Intersect(reportedConfigurationProfiles, new WifiProfileConfigurationNameComparer());
                // Create list of profiles for SystemConfigurator based on what NEEDS to be done
                var adjustedConfig = new WifiConfiguration() { ApplyFromDeviceTwin = desiredConfiguration.ApplyFromDeviceTwin, ReportToDeviceTwin = desiredConfiguration.ReportToDeviceTwin };
                adjustedConfig.Profiles = needToRemove.Union(needToAdd).ToList();

                // Only make changes if needed
                if (adjustedConfig.Profiles.Count != 0)
                {
                    // Download profiles needed for adding from Azure and load XML into WifiProfileConfiguration.Xml
                    await UpdateConfigWithProfileXmlAsync(connectionString, needToAdd);

                    try
                    {
                        // Let SystemConfigurator do the actual work
                        var request = new Message.SetWifiConfigurationRequest(adjustedConfig);
                        await _systemConfiguratorProxy.SendCommandAsync(request);
                    }
                    catch (Exception e)
                    {
                        Debug.WriteLine($"SetWifiConfigurationRequest failed: {e}");
                        // response = new { response = "rejected:", reason = e.Message };
                    }
                }

                if (desireReported)
                {
                    var configToUpdateTwin = await GetWifiConfigurationAsync();
                    var profilesToReport = configToUpdateTwin.Configuration.Profiles;
                    if (needToRemove != null)
                    {
                        foreach (var removed in needToRemove)
                        {
                            configToUpdateTwin.Configuration.Profiles.Add(removed);
                        }
                    }

                    var jsonToReport = configToUpdateTwin.Configuration.ToJson(ConfigurationType.Reported);
                    await this._callback.ReportPropertiesAsync(JsonSectionName, JObject.Parse(jsonToReport.ToString()));
                }
            }
        }

        private async Task HandleDesiredPropertyChangeAsync(JToken desiredValue)
        {
            var desiredConfig = WifiConfiguration.Parse(desiredValue.ToString(), ConfigurationType.Desired);
            string connectionString = _connectionString;
            await ProcessDesiredWifiConfigurationAsync(connectionString, desiredConfig);
        }

        // IClientPropertyHandler
        public async Task<CommandStatus> OnDesiredPropertyChange(JToken desiredValue)
        {
            await HandleDesiredPropertyChangeAsync(desiredValue);

            return CommandStatus.Committed;
        }

        // IClientPropertyHandler
        public async Task<JObject> GetReportedPropertyAsync()
        {
            Message.GetWifiConfigurationResponse reportedProperties = await GetWifiConfigurationAsync();
            return JObject.FromObject(reportedProperties);
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

        public struct GetWifiProfileDetailsParams
        {
            public string profileName;
            public string connectionString;
            public string containerName;
            public string blobName;
        }

        private async Task DoGetWifiProfileDetailsAsync(string jsonParam)
        {
            GetWifiProfileDetailsParams parameters = JsonConvert.DeserializeObject<GetWifiProfileDetailsParams>(jsonParam);

            var request = new Message.GetWifiDetailsRequest();
            request.profileName = parameters.profileName;

            Message.GetWifiDetailsResponse response = await _systemConfiguratorProxy.SendCommandAsync(request) as Message.GetWifiDetailsResponse;

            var info = new Message.AzureFileTransferInfo()
            {
                ConnectionString = parameters.connectionString,
                ContainerName = parameters.containerName,
                BlobName = parameters.blobName,
                Upload = true,
                LocalPath = ""
            };

            var appLocalDataFile = await ApplicationData.Current.TemporaryFolder.CreateFileAsync(parameters.blobName, CreationCollisionOption.ReplaceExisting);
            using (StreamWriter writer = new StreamWriter(await appLocalDataFile.OpenStreamForWriteAsync()))
            {
                await writer.WriteAsync(response.Xml);
            }
            await IoTDMClient.AzureBlobFileTransfer.UploadFile(info, appLocalDataFile);

            await appLocalDataFile.DeleteAsync();
        }

        private Task<string> GetWifiProfileDetailsAsync(string jsonParam)
        {
            var response = new { response = "succeeded", reason = "" };
            try
            {
                // Submit the work and return immediately.
                DoGetWifiProfileDetailsAsync(jsonParam).FireAndForget();
            }
            catch (Exception e)
            {
                response = new { response = "rejected:", reason = e.Message };
            }

            return Task.FromResult(JsonConvert.SerializeObject(response));
        }

        private string _connectionString;
        private ISystemConfiguratorProxy _systemConfiguratorProxy;
        private IClientHandlerCallBack _callback;
    }
}
