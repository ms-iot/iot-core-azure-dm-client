# Device management setup

Device management consists of several components:

1. **Device Application** - this UWP application runs on the device providing device functionaliry.
1. **IoTDMClient** - the Device Application links to this library.  It is repsonsible for handling/coordinating all DM requests. It runs on the device and is deployed with Device Application.
1. **CommProxy** - this native executable is responsible for delegating admin-level tasks for IotDMClient.  It runs on the device and should be built into the OS.
1. **SystemConfigurator** - this native service is responsible for handling admin-level tasks.  It runs on the device and should be built into the OS.
1. **IotHub DeviceTwin** - this is a logical representation of the device in Azure IotHub.
1. **Device Dashboard** - this is a conceptual component that manages devices.  This can run anywhere depending on OEM implementation.

## dm-proto-private

Microsoft has created a sample implementation for each of these parts in ms-iot/dm-proto-private.  This will provide the foundation to build fully functional Device Management into your device.

Here is a high level description of what is there:

1. **Toaster** - a foreground UI *Device Application* representing a fictional smart toaster.  In this case, device management is a part of the *Device Application*.  This is a UWP app that can be deployed from Visual Studio (using either Build > Deploy Solution or F5).
1. **IoTDMBackground** - a Background Application enabling simple device managment independent of any UI app.  In this case the *Device Application* is totally separated from the device management.  This is a UWP Background Application that can be deployed from Visual Studio (using either Build > Deploy Solution or F5).
1. **DMDashboard** - a *Device Dashboard* allowing registered smart toasters to be managed.  This is a .NET desktop application that can be built and run from Visual Studio.

## Development setup

To leverage these items on your IotCore device, follow these steps (steps below assume Toaster, but will work if applied to IoTDMBackground as well):

1. Setup your `Azure IotHub` account.
1. Install `Visual Studio 2015 Update 3` (be sure UWP features are enabled)
1. Setup your IotCore device using `IoT Dashboard`
1. Download the [dm-proto-private code](https://github.com/ms-iot/dm-proto-private) from github and open `IoTDM.sln` in Visual Studio
    + *[until TPM implementation]* Update samples\Toaster\ConnectionStringProvider.cs to use appropriate IotHub device connection string
1. Configure the Toaster project as the StartUp project by selecting Toaster in the Solution Explorer and selecting `Set as StartUp Project` in the Project  menu (this option is also available in the Solution Explorer's right click context menu).
1. Set the Configuration to x86 and `Build > Rebuild Solution`
1. Set the Configuration as required by your device and `Build > Rebuild Solution`
1. Deploy to your device by selecting Remote Machine, entering your device's IP address (or name), and `Build > Deploy Solution`.
1. Using SSH (or PowerShell) connect to your device as Administrator.
1. Verify that **SystemConfigurator.exe** and **CommProxy.exe** are present:

        Toaster with Debug ARM configuration: C:\Data\Users\DefaultAccount\AppData\Local\DevelopmentFiles\ToasterUiVS.Debug_ARM.msft
        IoTDMBackground with Debug ARM configuration: C:\Data\Users\DefaultAccount\AppData\Local\DevelopmentFiles\23983CETAthensQuality.IoTDMBackgroundSampleVS.Debug_ARM.msft

1. From `SSH` shell, run (assuming Toaster app with Debug ARM configuration, for IoTDMBackground use path specified in previous step):

        cd C:\Data\Users\DefaultAccount\AppData\Local\DevelopmentFiles\ToasterUiVS.Debug_ARM.msft
        net stop SystemConfigurator
        SystemConfigurator.exe -remove
        SystemConfigurator.exe -install
        net start SystemConfigurator

1. From `Visual Studio`, use F5 to launch Toaster app.  This should bring the **Toaster** app to your device and start it.
1. Run **DMDashboard** on your desktop by launching DMDashboard.exe
    + DMDashboard can trigger reboot in device, get and change time info, get and change device status, and install an App by using Azure blob storage.

## OEM device setup

The major differences between the Development and OEM setup are:

1. **CommProxy.exe** and **SystemConfiguration.exe** should not be placed in the *Device Application* folder.  We recommend `C:\Windows\System32`.
    + The reference in *IoTDMClientLib* to *CommProxy* must be updated in `SystemConfiguratorProxy.cs` to specify the new location.

            var processLauncherResult = await ProcessLauncher.RunToCompletionAsync(@"C:\Windows\System32\CommProxy.exe", "", processLauncherOptions);

    + **CommProxy.exe** must be added to the ProcessLauncher whitelist in the device's registry.

        At runtime, you can do this by running this command from `SSH` or `PowerShell`:

            reg.exe ADD "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\EmbeddedMode\ProcessLauncher" /v AllowedExecutableFilesList /t REG_MULTI_SZ /d "c:\windows\system32\commproxy.exe\0"

        You can build that into the OS, but creating a man file with this in it:

            <registryKeys>  
                <registryKey keyName="HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\EmbeddedMode\ProcessLauncher">  
                    <registryValue  
                        name="AllowedExecutableFilesList"  
                        value="$(runtime.system32)\CommProxy.exe"  
                        valueType="REG_MULTI_SZ"  
                        />  
                </registryKey>  
            </registryKeys>

1. **SystemConfigurator.exe** should be configured as a service in the image.  You can define this during image creation by including this in a man file:

        <memberships>
            <categoryMembership>
                <id
                    name="Microsoft.Windows.Categories.Services"
                    typeName="Service"
                    />
                <categoryInstance
                    subcategory="systemconfigurator"
                    >
                    <serviceData
                        dependOnService="rpcss,http"
                        description="SystemConfigurator"
                        displayName="SystemConfigurator"
                        errorControl="normal"
                        imagePath="%systemroot%\system32\systemconfigurator.exe"
                        name="systemconfigurator"
                        objectName="LocalSystem"
                        requiredPrivileges="SeTcbPrivilege,SeIncreaseBasePriorityPrivilege,SeCreatePermanentPrivilege,SeSecurityPrivilege,SeChangeNotifyPrivilege,SeImpersonatePrivilege,SeCreateGlobalPrivilege,SeAssignPrimaryTokenPrivilege,SeRestorePrivilege,SeTakeOwnershipPrivilege,SeBackupPrivilege,SeCreateSymbolicLinkPrivilege"
                        sidType="unrestricted"
                        start="auto"
                        startAfterInstall="synchronous"
                        type="win32OwnProcess"
                        >
                        <failureActions resetPeriod="900">
                        <actions>
                            <action
                                delay="120000"
                                type="restartService"
                                />
                            <action
                                delay="300000"
                                type="restartService"
                                />
                            <action
                                delay="0"
                                type="none"
                                />
                        </actions>
                        </failureActions>
                    </serviceData>
                </categoryInstance>
            </categoryMembership>
        </memberships>



