using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Microsoft.Devices.Management
{
    public enum SystemRebootRequestResponse
    {
        StartNow,
        AskAgainLater
    }

    public class ApplicationInfo
    {
        public string ApplicationName;
        public string PackageFamilyName;
    }

    public interface IDeviceManagementRequestHandler
    {
        Task<ApplicationInfo> GetApplicationInfo();

        Task<SystemRebootRequestResponse> IsSystemRebootAllowed();
    }
}