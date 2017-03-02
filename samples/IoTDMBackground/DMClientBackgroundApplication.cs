using Windows.ApplicationModel.Background;
using Microsoft.Azure.Devices.Client;
using Microsoft.Devices.Management;
using System.Threading.Tasks;
using Windows.ApplicationModel;
using Microsoft.Azure.Devices.Shared;

// The Background Application template is documented at http://go.microsoft.com/fwlink/?LinkID=533884&clcid=0x409

namespace IoTDMBackground
{
    class DeviceManagementRequestHandler : IDeviceManagementRequestHandler
    {
        public DeviceManagementRequestHandler()
        {
        }

        Task<ApplicationInfo> IDeviceManagementRequestHandler.GetApplicationInfo()
        {
            var appInfo = new ApplicationInfo
            {
                ApplicationName = Package.Current.DisplayName,
                PackageFamilyName = Package.Current.Id.FamilyName
            };

            return Task<ApplicationInfo>.FromResult(appInfo);
        }

        // It is always ok to reboot
        Task<SystemRebootRequestResponse> IDeviceManagementRequestHandler.IsSystemRebootAllowed()
        {
            return Task.FromResult(SystemRebootRequestResponse.Accept);
        }
    }

    public sealed class DMClientBackgroundApplication : IBackgroundTask
    {
        private DeviceManagementClient _dmClient;
        private BackgroundTaskDeferral _deferral;

        public async void Run(IBackgroundTaskInstance taskInstance)
        {
            _deferral = taskInstance.GetDeferral();

            // TODO: Get ConnectionString from TPM.  Use slot 0
            // TpmDevice tpm = new TpmDevice(0);
            // string deviceConnectionString = await tpm.GetConnectionString();

            string deviceConnectionString = "<connection string>";

            // Create DeviceClient. Application uses DeviceClient for telemetry messages, device twin
            // as well as device management
            DeviceClient deviceClient = DeviceClient.CreateFromConnectionString(deviceConnectionString, TransportType.Mqtt);

            // IDeviceTwin abstracts away communication with the back-end.
            // AzureIoTHubDeviceTwinProxy is an implementation of Azure IoT Hub
            IDeviceTwin deviceTwinProxy = new AzureIoTHubDeviceTwinProxy(deviceClient);

            // IDeviceManagementRequestHandler handles device management-specific requests to the app,
            // such as whether it is OK to perform a reboot at any givem moment, according the app business logic
            // ToasterDeviceManagementRequestHandler is the Toaster app implementation of the interface
            IDeviceManagementRequestHandler appRequestHandler = new DeviceManagementRequestHandler();

            // Create the DeviceManagementClient, the main entry point into device management
            _dmClient = await DeviceManagementClient.CreateAsync(deviceTwinProxy, appRequestHandler);

            // Set the callback for desired properties update. The callback will be invoked
            // for all desired properties -- including those specific to device management
            await deviceClient.SetDesiredPropertyUpdateCallback(OnDesiredPropertyUpdate, null);
        }

        private async Task OnDesiredPropertyUpdate(TwinCollection desiredProperties, object userContext)
        {
            // Let the device management client process properties specific to device management
            _dmClient.ProcessDeviceManagementProperties(desiredProperties);
        }
    }
}
