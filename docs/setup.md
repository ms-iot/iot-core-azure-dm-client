# Device management setup

Device management consists of several components:

1. Device Application - this UWP application runs on the device providing device functionaliry.
1. IoTDMClient - the Device Application links to this library.  It is repsonsible for handling/coordinating all DM requests. It runs on the device and is deployed with Device Application.
1. CommProxy - this native executable is responsible for delegating admin-level tasks for IotDMClient.  It runs on the device and should be built into the OS.
1. SystemConfigurator - this native service is responsible for handling admin-level tasks.  It runs on the device and should be built into the OS.
1. IotHub DeviceTwin - this is a logical representation of the device in Azure IotHub.
1. Device Dashboard - this is a conceptual component that manages devices.  This can run anywhere depending on OEM implementation.

## dm-proto-private

Microsoft has created a sample implementation for each of these parts in ms-iot/dm-proto-private.  This will provide the foundation to build fully functional Device Management into your device.

Here is a high level description of what is there:

1. Toaster - a Device Application representing a fictional smart toaster.  This is a UWP app that can be deployed from Visual Studio (using either Build > Deploy Solution or F5).
1. DMDashboard - a Device Dashboard allowing registered smart toasters to be managed.  This is a .NET desktop application that can be built and run from Visual Studio.

## Development setup

To leverage these items on your IotCore device, follow these steps:

1. Setup your Azure IotHub account.
1. Install Visual Studio 2015 Update 3 (be sure UWP features are enabled)
1. Setup your IotCore device using IoT Dashboard and using IoT Dashboard, connect your device to Azure
1. Download the dm-proto-private code from github and open IoTDM.sln in Visual Studio
    + *[until TPM implementation]* Update samples\Toaster\ConnectionStringProvider.cs to use appropriate IotHub device connection string
1. Set the Configuration to x86 and Build > Rebuild Solution
1. Set the Configuration as required by your device and Build > Rebuild Solution
1. Deploy to your device by selecting Remote Machine, entering your device's IP address (or name), and Build > Deploy Solution.
1. Using SSH (or PowerShell) connect to your device as Administrator.
1. Verify that SystemConfigurator.exe and CommProxy.exe are in this folder (assuming ARM configuration): C:\Data\Users\DefaultAccount\AppData\Local\DevelopmentFiles\ToasterUiVS.Debug_ARM.msft
1. From SSH shell, run (assuming ARM configuration):

        cd C:\Data\Users\DefaultAccount\AppData\Local\DevelopmentFiles\ToasterUiVS.Debug_ARM.msft
        SystemConfigurator.exe -install
        net start SystemConfigurator

1. From Visual Studio, use F5 to launch Toaster app.  This should bring Toaster UI to your device.
1. Run DMDashboard on your desktop by launching DMDashboard.exe
    + DMDashboard can trigger reboot in device, get and change time info, get and change device status, and install an App by using Azure blob storage.

## OEM device setup

The major differences between the Development and OEM setup are:

1. CommProxy.exe and SystemConfiguration.exe should not be placed in the Device Application folder.  We recommend C:\Windows\System32.
    + The reference in IoTClientDM to CommProxy must be updated in SystemConfiguratorProxy.cs to specify the new location.

            var processLauncherResult = await ProcessLauncher.RunToCompletionAsync(@"C:\Windows\System32\CommProxy.exe", "", processLauncherOptions);

    + CommProxy.exe must be added to the ProcessLauncher whitelist.

        At runtime, you can do this by running this command from SSH or PowerShell:

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

1. SystemConfigurator.exe should be configured as a service in the image.  You can define this during image creation by including this in a man file:

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
                        description="@%SystemRoot%\system32\systemconfigurator.dll,-2"
                        displayName="@%SystemRoot%\system32\systemconfigurator.dll,-1"
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



