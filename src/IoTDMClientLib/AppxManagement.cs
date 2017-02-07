using Microsoft.Devices.Management.Message;
using Microsoft.Devices.Management;
using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using Windows.Storage;

namespace IoTDMClient
{
    internal class AppBlobInfo
    {
        public string PackageFamilyName { get; set; }
        public BlobInfo Appx { get; set; }
        public List<BlobInfo> Dependencies { get; set; }

        public async Task<string> AppInstallAsync(DeviceManagementClient client)
        {
            var cleanup = new List<string>();
            var result = "install failed";
            try
            {
                var appInstallInfo = new AppInstallInfo();

                foreach (var dependencyBlobInfo in Dependencies)
                {
                    var depPath = await dependencyBlobInfo.DownloadToTemp();
                    appInstallInfo.Dependencies.Add(depPath);
                    cleanup.Add(dependencyBlobInfo.BlobName);
                }

                var path = await Appx.DownloadToTemp();
                appInstallInfo.AppxPath = path;

                appInstallInfo.PackageFamilyName = PackageFamilyName;
                await client.InstallAppAsync(appInstallInfo);

                result = "install succeeded";
            }
            catch (Exception e)
            {
                result += (": " + e.Message);
            }

            var response = JsonConvert.SerializeObject(new { response = result });
            return response;
        }
    }
}
