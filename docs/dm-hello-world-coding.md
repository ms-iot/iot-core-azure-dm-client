# DM Hello World Application
## Coding


#### Make Sure Library Is Ready
- Follow the steps described in [Building the Device Management Binaries](building-the-dm-binaries.md).
- Make sure you have the `c:\iot-core-azure-dm-client\nuget\IoTDMClientLib.1.2.0.nupkg` from the previous step.

#### Create A New Project
- Open Visual Studio, create a new `Blank App (Universal Windows)` C# Project. Let's name it `DMHelloWorld`.

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
- In Visual Studio, open `Tools | NuGet Package Manager | Package Manager Console`. In the NuGet console:
  - Run `Install-Package Microsoft.Azure.Devices.Client`
  - Run `Install-Package IoTDMClientLib -source nuget_path`
      - where <i>nuget_path</i> is the path to where the nuget was generated from following the [Building the Device Management Binaries](building-the-dm-binaries.md) step.
- Verify the references have been added under the project's `References' node in Solution Explorer or by looking at the project.json file.

#### Connect to Azure DM

- In MainPage.xaml.cs, add the following namespaces:
<pre>
using System.Threading.Tasks;
using Microsoft.Azure.Devices.Client;
using Microsoft.Azure.Devices.Shared;
using Microsoft.Devices.Management;
using Windows.Foundation.Diagnostics;
</pre>

- The DM library needs a way to callback into the application to handle certain DM requests (like "is it okay to reboot now?"). 
  To do that, the application needs to implement `IDeviceManagementRequestHandler`. Here's one possible implementation you can add to MainPage.xaml.cs:

<pre>
    class DMRequestHandler : IDeviceManagementRequestHandler
    {
        MainPage mainPage;

        public DMRequestHandler(MainPage mainPage)
        {
            this.mainPage = mainPage;
        }

        // Answer the question "is it OK to reboot the device"
        async Task&lt;bool&gt; IDeviceManagementRequestHandler.IsSystemRebootAllowed()
        {
            bool answer = await this.mainPage.IsSystemRebootAllowed();
            return answer;
        }
    }
</pre>

- And let's not forget to add an implementation for the callback (`IsSystemRebootAllowed`) in the MainPage:

<pre>
    public async Task&lt;bool&gt; IsSystemRebootAllowed()
    {
        return true;
    }
</pre>

- Now, let's code the part that connects the app to Azure, and wires in the DM library:

<pre>
    DeviceManagementClient deviceManagementClient;

    private async Task&lt;string&gt; GetConnectionStringAsync()
    {
        var tpmDevice = new TpmDevice(0);

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

    private async Task ResetConnectionAsync(DeviceClient existingConnection)
    {
        Logger.Log("ResetConnectionAsync start", LoggingLevel.Verbose);

        // Attempt to close any existing connections before
        // creating a new one
        if (existingConnection != null)
        {
            await existingConnection.CloseAsync().ContinueWith((t) =>
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
        var newDeviceClient = DeviceClient.CreateFromConnectionString(deviceConnectionString, TransportType.Mqtt);

        // IDeviceTwin abstracts away communication with the back-end.
        // AzureIoTHubDeviceTwinProxy is an implementation of Azure IoT Hub
        IDeviceTwin deviceTwin = new AzureIoTHubDeviceTwinProxy(newDeviceClient, ResetConnectionAsync, Logger.Log);

        // IDeviceManagementRequestHandler handles device management-specific requests to the app,
        // such as whether it is OK to perform a reboot at any givem moment, according to the app 
        // business logic.
        // DMRequestHandler is the Toaster app implementation of the interface
        IDeviceManagementRequestHandler appRequestHandler = new DMRequestHandler(this);

        // Create the DeviceManagementClient, the main entry point into device management
        var newDeviceManagementClient = await DeviceManagementClient.CreateAsync(deviceTwin, appRequestHandler);

        // Set the callback for desired properties updates. The callback will be invoked
        // for all desired properties changes in the device twin -- including those specific 
        // to Windows IoT Core device management.
        await newDeviceClient.SetDesiredPropertyUpdateCallbackAsync(OnDesiredPropertyUpdate, null);

        // Tell the deviceManagementClient to sync the device with the current desired state.
        await newDeviceManagementClient.ApplyDesiredStateAsync();

        this.deviceManagementClient = newDeviceManagementClient;
        Logger.Log("ResetConnectionAsync end", LoggingLevel.Verbose);
    }

    private async Task InitializeDeviceClientAsync()
    {
        while (true)
        {
            try
            {
                await ResetConnectionAsync(null);
                break;
            }
            catch (Exception e)
            {
                var msg = "InitializeDeviceClientAsync exception: " + e.Message + "\n" + e.StackTrace;
                System.Diagnostics.Debug.WriteLine(msg);
                Logger.Log(msg, LoggingLevel.Error);
            }

            await Task.Delay(5 * 60 * 1000);
        }
    }
</pre>

  - And of course, add the implementation for `OnDesiredPropertyUpdate` - which will handle Device Twin desired property changes.

<pre>
    public Task OnDesiredPropertyUpdate(TwinCollection desiredProperties, object userContext)
    {
        // Let the device management client process properties specific to device management
        this.deviceManagementClient.ApplyDesiredStateAsync(desiredProperties);

        // Application developer can process all the top-level nodes here
        return Task.CompletedTask;
    }
</pre>

- Finally, we call our connection function in the MainPage() constructor:

<pre>
    public MainPage()
    {
        this.InitializeComponent();
        this.InitializeDeviceClientAsync();
    }
</pre>

Now you have a fully functional DM client integrated into the application!

**Next Step**:

- Deploying the [DM Hello World! Application](dm-hello-world-deploying.md).

----

[Home Page](../README.md) | [DM Hello World Application](dm-hello-world-overview.md)