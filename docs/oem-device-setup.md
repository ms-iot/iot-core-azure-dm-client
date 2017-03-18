# OEM Device Setup

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



