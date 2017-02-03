using Microsoft.Devices.Management.Message;
using Microsoft.Devices.Management;
using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using Windows.Storage;

namespace IoTDMClient
{
    public class AppBlobInfo
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

                // Dependencies
                {
                    foreach (var dependencyBlobInfo in Dependencies)
                    {
                        var path = await dependencyBlobInfo.DownloadToTemp();
                        appInstallInfo.Dependencies.Add(path);
                        cleanup.Add(dependencyBlobInfo.BlobName);
                    }
                }

                // Appx
                {
                    var path = await Appx.DownloadToTemp();
                    appInstallInfo.AppxPath = path;
                    cleanup.Add(Appx.BlobName);
                }

                appInstallInfo.PackageFamilyName = PackageFamilyName;
                await client.InstallAppAsync(appInstallInfo);

                result = "install succeeded";
            }
            catch (Exception e)
            {

            }
            finally
            {
                foreach (var file in cleanup)
                {
                    var fileTask = ApplicationData.Current.TemporaryFolder.GetFileAsync(file).AsTask();
                    fileTask.Wait();
                    StorageFile fDelete = fileTask.Result;
                    var deleteTask = fDelete.DeleteAsync().AsTask();
                    deleteTask.Wait();
                }
            }

            var response = JsonConvert.SerializeObject(new { response = result });
            return response;
        }
    }

    public class AppxManagement
    {
    }
}
