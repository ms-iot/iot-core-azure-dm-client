# DM Hello World Application
## Coding


#### Make Sure Library Is Ready
- Follow the steps described in [Building the Device Management Binaries](building-the-dm-binaries.md).
- Make sure you have the `c:\iot-core-azure-dm-client\nuget\IoTDMClientLib.1.4.4.nupkg` from the previous step.

#### Create A New Project
- Open Visual Studio, create a new `Blank App (Universal Windows)` C# Project. Let's name it `DMHelloWorld`.
- Make sure it is building using **SDK min version 16299**.

#### Add the systemManagement Capability
- Expand the project node in the Solution Explorer.
- Right-click the `Package.appxmanifest` and select `Open With`.
- Select `XML (Text) Editor` and hit `Ok`.
- Add the following namespace to the `Package` tag:
  - `xmlns:iot="http://schemas.microsoft.com/appx/manifest/iot/windows10"`
- Add `iot` to the list of `IgnorableNamespaces`. It should look something like this:
  - `IgnorableNamespaces="uap mp iot"`
- Add the `systemManagement` capability to the `Capabilities` element:
  - `<iot:Capability Name="systemManagement" />`
- Make sure it builds successfully.

#### Add References to the DM Libraries
- If you are using VS 2015, from Solution Explorer, open `project.json`.
  - Update the version number of `Microsoft.NETCore.UniversalWindowsPlatform` to `"5.2.2"`.
  - Note: If you are using VS 2017, ignore this step.
- In Visual Studio, `Tools | NuGet Package Manager | Package Manager Settings`.
    - Expand the `NuGet Package Manager` node and select `Package Sources`.
    - Under `Available package sources`, add the path to the nuget package you have just built (for the example above, it'll be: c:\iot-core-azure-dm-client\nuget).
- In Visual Studio, open `Tools | NuGet Package Manager | Package Manager Console`. In the NuGet console:
  - Run `Install-Package Microsoft.Azure.Devices.Client -Version 1.4.0`
    - Note: 1.4.1 and 1.4.2 cause a build error ([open issue](https://github.com/ms-iot/iot-core-azure-dm-client/issues/174)).
  - Run `Install-Package IoTDMClientLib -source nuget_path`
      - where <i>nuget_path</i> is the path to where the nuget was generated from following the [Building the Device Management Binaries](building-the-dm-binaries.md) step.
- Verify the references have been added under the project's `References' node in Solution Explorer or by looking at the project.json file.

#### Connect to Azure DM

- In MainPage.xaml.cs, add the following namespaces:
<pre>
using System.Threading;
using System.Threading.Tasks;
using Microsoft.Azure.Devices.Client;
using Microsoft.Azure.Devices.Shared;
using Microsoft.Devices.Management;
using Windows.Foundation.Diagnostics;
</pre>

- The DM library needs a way to callback into the application to handle certain DM requests (like "is it okay to reboot now?"). 
  To do that, the application needs to implement `IDeviceManagementRequestHandler`. Here's one possible implementation you can add to MainPage.xaml.cs:

<pre>
    class AppDeviceManagementRequestHandler : IDeviceManagementRequestHandler
    {
        MainPage mainPage;

        public AppDeviceManagementRequestHandler(MainPage mainPage)
        {
            this.mainPage = mainPage;
        }

        // Answer the question "is it OK to reboot the device"
        async Task&lt;bool&gt; IDeviceManagementRequestHandler.IsSystemRebootAllowed()
        {
            return true;
        }
    }
</pre>

- Now, let's code the part that connects the app to Azure, and wires in the DM library:

<pre>
    private DeviceManagementClient _deviceManagementClient;
    private EventWaitHandle _iotHubOfflineEvent;
    private DeviceClient _deviceClient;

    private async Task&lt;string&gt; GetConnectionStringAsync()
    {
        var tpmDevice = new TpmDevice();

        string connectionString = "";

        do
        {
            try
            {
                connectionString = await tpmDevice.GetConnectionStringAsync();
                break;
            }
            catch (Exception)
            {
                // We'll just keep trying.
            }
            await Task.Delay(1000);

        } while (true);

        return connectionString;
    }

    private async Task ResetConnectionAsync()
    {
        Logger.Log("ResetConnectionAsync start", LoggingLevel.Verbose);
        // Attempt to close any existing connections before
        // creating a new one
        if (_deviceClient != null)
        {
            await _deviceClient.CloseAsync().ContinueWith((t) =>
            {
                var e = t.Exception;
                if (e != null)
                {
                    var msg = "existingClient.CloseAsync exception: " + e.Message + "\n" + e.StackTrace;
                    System.Diagnostics.Debug.WriteLine(msg);
                    Logger.Log(msg, LoggingLevel.Verbose);
                }
            });
        }

        // Get new SAS Token
        var deviceConnectionString = await GetConnectionStringAsync();

        // Create DeviceClient. Application uses DeviceClient for telemetry messages, device twin
        // as well as device management
        _deviceClient = DeviceClient.CreateFromConnectionString(deviceConnectionString, TransportType.Mqtt);

        // IDeviceTwin abstracts away communication with the back-end.
        // AzureIoTHubDeviceTwinProxy is an implementation of Azure IoT Hub
        IDeviceTwin deviceTwin = new AzureIoTHubDeviceTwinProxy(_deviceClient, _iotHubOfflineEvent, Logger.Log);

        // IDeviceManagementRequestHandler handles device management-specific requests to the app,
        // such as whether it is OK to perform a reboot at any givem moment, according the app business logic
        // AppDeviceManagementRequestHandler is the Toaster app implementation of the interface
        IDeviceManagementRequestHandler appRequestHandler = new AppDeviceManagementRequestHandler(this);

        // Create the DeviceManagementClient, the main entry point into device management
        this._deviceManagementClient = await DeviceManagementClient.CreateAsync(deviceTwin, appRequestHandler);

        // Set the callback for desired properties update. The callback will be invoked
        // for all desired properties -- including those specific to device management
        await _deviceClient.SetDesiredPropertyUpdateCallbackAsync(OnDesiredPropertyUpdated, null);

        // Tell the _deviceManagementClient to sync the device with the current desired state.
        await this._deviceManagementClient.ApplyDesiredStateAsync();

        Logger.Log("ResetConnectionAsync end", LoggingLevel.Verbose);
    }

    private void InitializeDeviceClientAsync()
    {
        IAsyncAction asyncAction = Windows.System.Threading.ThreadPool.RunAsync(
            async (workItem) =>
            {
                while (true)
                {
                    _iotHubOfflineEvent.WaitOne();
                    try
                    {
                        await ResetConnectionAsync();
                    }
                    catch (Exception e)
                    {
                        _iotHubOfflineEvent.Set();

                        var msg = "InitializeDeviceClientAsync exception: " + e.Message + "\n" + e.StackTrace;
                        System.Diagnostics.Debug.WriteLine(msg);
                        Logger.Log(msg, LoggingLevel.Error);
                    }

                    await Task.Delay(5 * 60 * 1000);
                }
            });
    }
</pre>

  - And of course, add the implementation for `OnDesiredPropertyUpdate` - which will handle Device Twin desired property changes.

<pre>
    public async Task OnDesiredPropertyUpdated(TwinCollection twinProperties, object userContext)
    {
        Dictionary&lt;string, object&gt; desiredProperties = AzureIoTHubDeviceTwinProxy.DictionaryFromTwinCollection(twinProperties);

        // Let the device management client process properties specific to device management
        await this._deviceManagementClient.ApplyDesiredStateAsync(desiredProperties);
    }
</pre>

- Finally, we call our connection function in the MainPage() constructor:

<pre>
    public MainPage()
    {
        this.InitializeComponent();

        _iotHubOfflineEvent = new EventWaitHandle(true, EventResetMode.AutoReset);

        this.InitializeDeviceClientAsync();
    }
</pre>

Now you have a fully functional DM client integrated into the application!

**Next Step**:

- Deploying the [DM Hello World! Application](dm-hello-world-deploying.md).

----

[Home Page](../README.md) | [DM Hello World Application](dm-hello-world-overview.md)