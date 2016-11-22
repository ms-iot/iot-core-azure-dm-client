using System;
using System.Threading.Tasks;
using Windows.ApplicationModel;

using Microsoft.Devices.Management;

namespace Toaster
{
    class ToasterDeviceManagementRequestHandler : IDeviceManagementRequestHandler
    {
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
        Task<SystemRebootRequestResponse> IDeviceManagementRequestHandler.IsSystemRebootAllowed()
        {
            // TODO: ask the user
            return Task<SystemRebootRequestResponse>.FromResult(SystemRebootRequestResponse.AskAgainLater);
        }
    }
}
