using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net.Http;
using Windows.ApplicationModel.Background;
using Microsoft.Azure.Devices.Client;
using Microsoft.Devices.Management;
using System.Threading.Tasks;
using Windows.ApplicationModel;
using Microsoft.Azure.Devices.Shared;
using Windows.ApplicationModel.AppService;
using Windows.Foundation.Collections;

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
            var appInfo = // get info from any AppService connections
            new ApplicationInfo
            {
                ApplicationName = "IoTDMBackground",
                PackageFamilyName = Package.Current.Id.FamilyName
            };

            return Task<ApplicationInfo>.FromResult(appInfo);
        }

        // Answer the question "is it OK to reboot the toaster"
        async Task<SystemRebootRequestResponse> IDeviceManagementRequestHandler.IsSystemRebootAllowed()
        {
            bool answer = true;

            // get answer from any AppService connections
            foreach (var connection in DMClientBackgroundApplication.Instance.Connections)
            {
                var status = await connection.OpenAsync();
                if (status != AppServiceConnectionStatus.Success)
                {
                    var message = new ValueSet();
                    message.Add("get", "readyForReboot");
                    var result = await connection.SendMessageAsync(message);
                    if (result.Status == AppServiceResponseStatus.Success)
                    {
                        var response = result.Message;
                        bool readyForReboot = (bool)response["readyForReboot"];
                        if (!readyForReboot)
                        {
                            answer = false;
                            break;
                        }
                    }
                }
            }

            return answer ? SystemRebootRequestResponse.Accept : SystemRebootRequestResponse.Reject;
        }
    }

    public sealed class DMClientBackgroundApplication : IBackgroundTask
    {
        private readonly string DeviceConnectionString = ConnectionStringProvider.Value;

        private DeviceManagementClient _dmClient;
        private BackgroundTaskDeferral _deferral;

        public async void Run(IBackgroundTaskInstance taskInstance)
        {
            _deferral = taskInstance.GetDeferral();

            // Create DeviceClient. Application uses DeviceClient for telemetry messages, device twin
            // as well as device management
            DeviceClient deviceClient = DeviceClient.CreateFromConnectionString(DeviceConnectionString, TransportType.Mqtt);

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

        public DMClientBackgroundApplication()
        {
            Connections = new List<AppServiceConnection>();
            DMClientBackgroundApplication.Instance = this;
        }
        public static DMClientBackgroundApplication Instance { get; private set; }
        public IList<AppServiceConnection> Connections { get; private set; }

        internal async Task<string> ExecuteDMRequest(string request, string data)
        {
            try
            {
                if (request.Equals("checkForUpdates"))
                {
                    return (await _dmClient.CheckForUpdatesAsync()).ToString();
                }
                else if (request.Equals("immediateReboot"))
                {
                    await _dmClient.ImmediateRebootAsync();
                    return true.ToString();
                }
                else if (request.Equals("factoryReset"))
                {
                    await _dmClient.DoFactoryResetAsync();
                    return true.ToString();
                }
            } catch (Exception)
            {
            }
            return false.ToString();
        }

        private async Task OnDesiredPropertyUpdate(TwinCollection desiredProperties, object userContext)
        {
            // Let the device management client process properties specific to device management
            _dmClient.ProcessDeviceManagementProperties(desiredProperties);

            // Application developer can process all the top-level nodes here
            foreach (var connection in Connections)
            {
                var status = await connection.OpenAsync();
                if (status != AppServiceConnectionStatus.Success)
                {
                    var message = new ValueSet();
                    message.Add("desiredPropertyUpdate", desiredProperties.ToJson());
                    var result = await connection.SendMessageAsync(message);
                }
            }
        }
    }
}
