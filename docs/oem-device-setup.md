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

- The device management library used by the UWP application communicates with the NT Service, SystemConfigurator.exe, over a capability-protected RPC channel.  By default, this is configured to use the systemManagement 
capability.  Meaning that any UWP app using systemManagement that is running on the device can utilize this RPC channel.  If you want to further lock this down, you can request a Custom Capability from the store which 
will allow you to further secure this channel.  More information about Custom Capabilities can be found [here](https://github.com/Microsoft/Windows-universal-samples/tree/master/Samples/CustomCapability).

- To configure the `SystemConfigurator` service, create a cmd file and invoke it from the main configuration script `OEMCustomization.cmd` (which is called on every boot).

- Please also see example [AzureDM.Services.wm.xml](https://github.com/ms-iot/iot-adk-addonkit/blob/master/Source-arm/Packages/AzureDM.Services/AzureDM.Services.wm.xml) and [Create Windows Universal OEM Packages](https://docs.microsoft.com/en-us/windows-hardware/manufacture/iot/create-packages)

#### DMSetup.cmd

<pre>
    c:\windows\system32\systemconfigurator.exe -install
    c:\windows\system32\sc.exe config systemconfigurator start=auto
	c:\windows\system32\sc.exe failure systemconfigurator reset= 0 actions= restart/0/restart/0/restart/0
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

## Device time considerations

- In order for the device managagement library to be able to connect to the Azure IoT Hub, it is necessary that the oem device's time settings are configured properly. Certain devices (for example Raspberry Pi) lack the support for persistent time settings. This can however be configured by including an addition to the product's OEMCustomization.cmd as follows:

<pre>
    REM Force time sync on boot
	w32tm /resync /force
</pre>

- This addition assumes network connectivity on boot. A scheduled task could be added to force a time synchronisation periodically, for example:

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
	 
     REM Resync with time server every hour	
     schtasks /Create /SC HOURLY /TN TimeSyncEveryHour /TR "w32tm /resync /force" /RU "SYSTEM"
	 
     reg add HKLM\Software\IoT /v FirstBootDone /t REG_DWORD /d 1 /f >nul 2>&1 
) 

REM Force time sync on boot
w32tm /resync /force
</pre>

- It is advisable to set the correct timezone for the device as well, since Windows IoT Core defaults to US Pacific timezone, for example:

<pre>
     tzutil /s "W. Europe Standard Time"
</pre>


----

[Home Page](../README.md)