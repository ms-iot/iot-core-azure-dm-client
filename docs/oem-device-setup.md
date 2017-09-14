# OEM Device Setup

In a production environment, the OEM creates an OS image customized with exactly the components needed for the target device. 
The process and tools by which customized images can be created is described here [IoT Core Manufacturing Guide](https://docs.microsoft.com/en-us/windows-hardware/manufacture/iot/iot-core-manufacturing-guide).

In order to include Azure Device Management functionality in such images, 

- Build The Binaries
  - Follow the instructions [here](building-the-dm-binaries.md).

- Include The Binaries:
  - The UWP application that hosts the device management library (see the [Walk-Through](dm-hello-world-overview.md) on how to create one).
  - `SystemConfigurator.exe`

- Configure The Binaries:
  - `SystemConfigurator` service is installed, and is configured to start automatically.

## Including the Binaries

- OEM authors packages to include all the necessary binaries.
- `SystemConfigurator.exe` is to be placed in system32.

<pre>
    &lt;Files&gt;
    &lt;File Source="SystemConfigurator.exe" DestinationDir="$(runtime.system32)" /&gt;
    &lt;File Source="DMSetup.cmd" /&gt;
    &lt;/Files&gt;
</pre>

## Configuring the Binaries

- To configure the `SystemConfigurator` service, create a cmd file and invoke it from the main configuration script `OEMCustomization.cmd` (which is called on every boot).


- The device management library used by the UWP application communicates with the NT Service, SystemConfigurator.exe, over a capability-protected RPC channel.  By default, this is configured to use the systemManagement capability.  Meaning that any UWP app using systemManagement that is running on the device can utilize this RPC channel.  If you want to further lock this down, you can request a Custom Capability from the store which will allow you to further secure this channel.  More information about Custom Capabilities can be found [here](https://github.com/Microsoft/Windows-universal-samples/tree/master/Samples/CustomCapability).

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
     call DMSetup.cmd
     reg add HKLM\Software\IoT /v FirstBootDone /t REG_DWORD /d 1 /f >nul 2>&1 
) 
</pre>

(see more samples on authoring OEMCustomization.cmd [here](https://github.com/ms-iot/iot-adk-addonkit/blob/master/Source-arm/Products/))

----

[Home Page](../README.md)