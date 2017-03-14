using System;
using System.Threading.Tasks;
using Windows.ApplicationModel;

using Microsoft.Devices.Management;

namespace Toaster
{
    class ToasterDeviceManagementRequestHandler : IDeviceManagementRequestHandler
    {
        MainPage mainPage;

        public ToasterDeviceManagementRequestHandler(MainPage mainPage)
        {
            this.mainPage = mainPage;
        }

        Task<ApplicationInfo> IDeviceManagementRequestHandler.GetApplicationInfo()
        {
            var appInfo = new ApplicationInfo
            {
                ApplicationName = "Toasters International",
                PackageFamilyName = Package.Current.Id.FamilyName
            };

            return Task<ApplicationInfo>.FromResult(appInfo);
        }

        // Answer the question "is it OK to reboot the toaster"
        async Task<bool> IDeviceManagementRequestHandler.IsSystemRebootAllowed()
        {
            bool answer = await this.mainPage.YesNo("Allow reboot?");
            return answer;
        }
    }
}
