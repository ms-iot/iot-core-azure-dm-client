using Microsoft.Devices.Management;
using Microsoft.Devices.Management.Message;
using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Threading.Tasks;

namespace IoTDMClient
{
    internal class AppBlobInfo
    {
        public string PackageFamilyName { get; set; }
        public BlobInfo Appx { get; set; }
        public List<BlobInfo> Dependencies { get; set; }

        public async Task<string> AppInstallAsync(DeviceManagementClient client)
        {
            try
            {
                var appInstallInfo = new AppInstallInfo();
                foreach (var dependencyBlobInfo in Dependencies)
                {
                    var depPath = await dependencyBlobInfo.DownloadToTemp(client);
                    appInstallInfo.Dependencies.Add(depPath);
                }

                var path = await Appx.DownloadToTemp(client);
                appInstallInfo.AppxPath = path;

                appInstallInfo.PackageFamilyName = PackageFamilyName;
                await client.InstallAppAsync(appInstallInfo);

                var response = JsonConvert.SerializeObject(new { response = "succeeded" });
                return response;
            }
            catch (Exception e)
            {
                var response = JsonConvert.SerializeObject(new { response = "failed", reason = e.Message });
                return response;
            }

        }
    }
}
