using System.Threading.Tasks;

namespace Microsoft.Devices.Management
{
    // DM Client talks to the App via this interface. The App implements the interface
    public interface IDeviceManagementRequestHandler
    {
        Task<bool> IsSystemRebootAllowed();
    }
}