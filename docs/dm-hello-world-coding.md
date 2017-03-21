# DM Hello World Application
## Coding


#### Make Sure Library Is Ready
- Follow the steps described in [Building the Library](building-the-library.md)
- Make sure you have the `c:\iot-core-azure-dm-client\nuget\IoTDMClientLib.1.0.0.nupkg` from the previous step.

#### Create A New Project
- Open Visual Studio, create a new `Blank App (Universal Windows)` C# Project. Let's name it `DMHelloWorld`.

#### Add the systemManagement Capability
- Expand the project node in the Solution Explorer.
- Right-click the Package.appxmanifest and select `Open With`.
- Select `XML (Text) Editor` and hit `Ok`.
- Add the following namespace to the `Package` tag:
  - `xmlns:iot="http://schemas.microsoft.com/appx/manifest/iot/windows10"`
- Add `iot` to the list of `IgnorableNamespaces`. It should look something like this:
  - `IgnorableNamespaces="uap mp iot"`
- Add the `systemManagement` capability to the `Capabilities` element:
  - `<iot:Capability Name="systemManagement" />`
- Make sure it builds successfully.

#### Add References to the DM Libraries
- From Solution Explorer, open `project.json`.
  - Update the version number of `Microsoft.NETCore.UniversalWindowsPlatform` to `"5.2.2"`.
- In Visual Studio, open `Tools | NuGet Package Manager | Package Manager Console`. In the NuGet console:
  - Run `Install-Package Microsoft.Azure.Devices.Client`
  - Run `Install-Package IoTDMClientLib -source nuget_path`
      - where <i>nuget_path</i> is the path to where the nuget was generated from following the [Building the Library](building-the-library.md) step.
- Verify the references have been added under the project's `References' node in Solution Explorer or by looking at the project.json file.

#### Connect to Azure DM

- In MainPage.xaml.cs, add the following namespaces:
<pre>
using System.Threading.Tasks;
using Windows.ApplicationModel;

using Microsoft.Azure.Devices.Client;
using Microsoft.Azure.Devices.Shared;

using Microsoft.Devices.Management;
</pre>

- The DM library needs a way to callback into the application to handle certain DM requests (like "is rebooting now okay?"). 
  To do, the application needs to implement `IDeviceManagementRequestHandler`. Here's one possible implementation you can add to MainPage.xaml.cs:

<pre>
    class DMRequestHandler : IDeviceManagementRequestHandler
    {
        MainPage mainPage;

        public DMRequestHandler(MainPage mainPage)
        {
            this.mainPage = mainPage;
        }

        Task&lt;ApplicationInfo&gt; IDeviceManagementRequestHandler.GetApplicationInfo()
        {
            var appInfo = new ApplicationInfo
            {
                ApplicationName = "DMHelloWorld",
                PackageFamilyName = Package.Current.Id.FamilyName
            };

            return Task<ApplicationInfo>.FromResult(appInfo);
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

- Now, let's code the part that connects the app to Azure, and wires in the DM library too:

<pre>
    DeviceManagementClient deviceManagementClient;
    private readonly string DeviceConnectionString = "&lt;connection string&gt;";

    private async Task InitializeDeviceClientAsync()
    {
        // Create DeviceClient. Application uses DeviceClient for telemetry messages, device twin
        // as well as device management
        DeviceClient deviceClient = DeviceClient.CreateFromConnectionString(DeviceConnectionString, TransportType.Mqtt);

        // IDeviceTwin abstracts away communication with the back-end.
        // AzureIoTHubDeviceTwinProxy is an implementation of Azure IoT Hub
        IDeviceTwin deviceTwinProxy = new AzureIoTHubDeviceTwinProxy(deviceClient);

        // IDeviceManagementRequestHandler handles device management-specific requests to the app,
        // such as whether it is OK to perform a reboot at any givem moment, according to the app 
        // business logic.
        // DMRequestHandler is the Toaster app implementation of the interface
        IDeviceManagementRequestHandler appRequestHandler = new DMRequestHandler(this);

        // Create the DeviceManagementClient, the main entry point into device management
        this.deviceManagementClient = await DeviceManagementClient.CreateAsync(deviceTwinProxy, appRequestHandler);

        // Set the callback for desired properties update. The callback will be invoked
        // for all desired properties -- including those specific to device management
        await deviceClient.SetDesiredPropertyUpdateCallback(OnDesiredPropertyUpdate, null);
    }
</pre>

- We also need to listen to DM requests as they come in and forward them to the DM library:
    - At the end of `InitializeDeviceClientAsync()` add:

<pre>
        // Set the callback for desired properties update. The callback will be invoked
        // for all desired properties -- including those specific to device management
        await deviceClient.SetDesiredPropertyUpdateCallback(OnDesiredPropertyUpdate, null);
</pre>

  - And of course, add the implementation for `OnDesiredPropertyUpdate` - which will handle Device Twin desired property changes.

<pre>
    public Task OnDesiredPropertyUpdate(TwinCollection desiredProperties, object userContext)
    {
        // Let the device management client process properties specific to device management
        this.deviceManagementClient.ProcessDeviceManagementProperties(desiredProperties);

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

Now you have a fully functional DM!

**Next Step**:

- Deploying the [DM Hello World! Application](dm-hello-world-deploying.md).
