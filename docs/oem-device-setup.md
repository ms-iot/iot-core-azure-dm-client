# OEM Device Setup

In a production environment, the OEM creates an OS image customized with exactly the components needed for the target device. 
The process and tools by which customized images can be created is described here [IoT Core Manufacturing Guide](https://docs.microsoft.com/en-us/windows-hardware/manufacture/iot/iot-core-manufacturing-guide).

In order to include Azure Device Management functionality in such images, 

- Include the following components:
  - The UWP application that hosts the device management library (see the [Walk-Through](dm-hello-world-overview.md) on how to create one).
  - `SystemConfigurator.exe`
  - `CommProxy.exe`

- Configure the componenents:
  - `CommProxy.exe` need to be whitelisted.
  - `SystemConfigurator` service is installed, and is configured to start automatically.

## Including the Binaries

- OEM authors packages to include all the necessary binaries.
- `CommProxy.exe` and `SystemConfigurator.exe` need to be placed in system32.

## Configuring the Binaries

- To white-list `CommProxy.exe`, use the following snippet into your `<package>.xml` definitions file:

<pre>
    &lt;RegKeys&gt;
        &lt;RegKey KeyName="$(hklm.software)\Microsoft\Windows\CurrentVersion\EmbeddedMode\ProcessLauncher"&gt;  
            &lt;RegValue  
                Name="AllowedExecutableFilesList"  
                Value="$(runtime.system32)\CommProxy.exe\0"  
                Type="REG_MULTI_SZ"  
                /&gt;  
        &lt;/RegKey&gt;  
    &lt;/RegKeys&gt;
</pre>

For more details on authoring registry configuration, see [Lab 1c: Add a file and a registry setting to an image](https://docs.microsoft.com/en-us/windows-hardware/manufacture/iot/add-a-registry-setting-to-an-image).

- To configure the `SystemConfigurator` service, create a cmd file and invoke it from the main configuration script `OEMCustomization.cmd` (which is called on every boot).

#### DMSetup.cmd

<pre>
    c:\windows\system32\systemconfigurator.exe -install
    c:\windows\system32\sc.exe config systemconfigurator start=auto
    net start systemconfigurator
</pre>

#### OEMCustomization.cmd

<pre>
@echo off 
REM OEM Customization Script file 
REM This script if included in the image, is called everytime the system boots. 

reg query HKLM\Software\IoT /v FirstBootDone >nul 2>&1 

if %errorlevel% == 1 ( 
     REM Enable Administrator User 
     net user Administrator p@ssw0rd /active:yes 
     reg add HKLM\Software\IoT /v FirstBootDone /t REG_DWORD /d 1 /f >nul 2>&1 
     call DMSetup.cmd
) 
</pre>

(see more samples on authoring OEMCustomization.cmd [here](https://github.com/ms-iot/iot-adk-addonkit/blob/master/Source-arm/Products/))







