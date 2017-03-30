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
                if (null != Dependencies)
                {                    
                    foreach (var dependencyBlobInfo in Dependencies)
                    {
                        var depPath = await dependencyBlobInfo.DownloadToTempAsync(client);
                        appInstallInfo.Dependencies.Add(depPath);
                    }
                }

                var path = await Appx.DownloadToTempAsync(client);
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
