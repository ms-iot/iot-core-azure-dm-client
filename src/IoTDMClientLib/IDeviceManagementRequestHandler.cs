using System.Threading.Tasks;

namespace Microsoft.Devices.Management
{
    public class ApplicationInfo
    {
        public string ApplicationName;
        public string PackageFamilyName;
    }

    // DM Client talks to the App via this interface. The App implements the interface
    public interface IDeviceManagementRequestHandler
    {
        Task<ApplicationInfo> GetApplicationInfo();

        Task<bool> IsSystemRebootAllowed();
    }
}