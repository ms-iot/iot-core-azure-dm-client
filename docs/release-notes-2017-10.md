# Release Notes - October 2017

## Target IoT Skus

This release targets both the **IoT Core** and the **IoT Enterprise** skus. However, not all features are supported on IoT Enterprise.

For the IoT Enterprise sku, the following extra setup steps need to take place:

1. Limpet.exe is present at c:\windows\system32 (copy from an IoT Core image system folder).
2. xperf.exe is present at c:\windows\system32 (copy from an IoT Core image system folder).
3. [Embedded Mode](https://docs.microsoft.com/en-us/windows/iot-core/develop-your-app/embeddedmode) is enabled.
4. Make sure the architecture of SystemConfigurator matches the OS architecture (i.e. on x64 OSes, deploy the x64 SystemConfigurator).

## Windows IoT Azure DM Client

This page contains a summary of all features the current Windows IoT Azure DM Client offers.

The features can be split into three main categories:

- **Area-Specific Features**
  - Application Management
  - Certificate Management
  - Device Health Attestation
  - Device Info
  - Diagnostic Logs
  - Factory Reset
  - Reboot Management
  - Time Settings
  - WiFi Settings
  - Windows Telemetry
  - Windows Update
- **Patterns**
  - IoT Hub Accessible Functionality
  - Device Accessible Functionality
  - Policy
  - Details Level
  - Error Reporting
- **Infrastructure Features**
  - Connectivity Restoration
  - Diagnosis
  - Inter-Process Security

## Area-Specific Features

**Note:** The json schema has changed for all features, even those which existing in the 2017 April release.

| Area | Supported Actions | New in Oct. 2017 | IoT Core | IoT Enterprise |
|:----|-----|-----|-----|-----|
| [Application Management](application-management.md)               | | | | |
|                        | Add/Update an arbitrary application (inc. self) | |Y| |
|                        | List store/non-store applications        |Y|Y| |
|                        | Configure startup applications           |Y|Y| |
|                        | Configure startup background applications|Y|Y| |
|                        | Start/stop applications                  |Y|Y| |
|                        | Uninstall applications                   |Y|Y| |
| | | | | |
| [Certificate Management](certificate-management.md)               | | | |
|                        | Add a certificate                        | |Y|Y|
|                        | List certificates                        | |Y|Y|
|                        | Export and Upload certificates           | |Y|Y|
|                        | Delete certificates                      |Y|Y|Y|
| | | | | |
| [Device Health Attestation](device-health-attestation.md)       | | | | |
|                        | Configure and trigger device Health Attestation remotely. |Y|Y| |
| | | | | |
| [Device Info](device-info.md)                                   | | | | |
|                        | Retrieve the device info.                |Y|Y|Y|
| | | | | |
| [Diagnostic Logs](diagnostic-logs-management.md)                | | | | |
|                        | Configure ETW collectors for any number of providers. |Y|Y|Y*|
|                        | Start/Stop collecting ETW.               |Y|Y|Y|
|                        | Upload generated ETW files.              |Y|Y|Y|
|                        | Delete generated ETW files from the device. |Y|Y|Y|
| | | | | |
| [Factory Reset](device-factory-reset.md)                        | | | | |
|                        | Reset the device and optionally clear the TPM. |Y|Y| |
| | | | | |
| [Reboot Management](reboot-management.md)                       | | | | |
|                        | Schedule reboots                         | |Y|Y|
|                        | Trigger immediate reboots                | |Y|Y|
|                        | Retrieve last reboot info                | |Y|Y|
|                        | Block system reboots                     | |Y|Y|
| | | | | |
| [Time Settings](time-management.md)                             | | | | |
|                        | Configure dynamic time zone              |Y|Y|Y|
|                        | Configure custom time zone               | |Y|Y|
|                        | Configure time server                    | |Y|Y|
|                        | Configure time service                   |Y|Y|Y|
| | | | | |
| [WiFi Settings](wifi-management.md)                             | | | | |
|                        | Add WiFi profiles. |Y|Y| |
|                        | List installed WiFi profiles. |Y|Y| |
|                        | Delete installed WiFi profiles. |Y|Y| |
|                        | Export/Upload WiFi profiles. |Y|Y| |
| | | | | |
| [Windows Telemetry](windows-telemetry-management.md)            | | | | |
|                        | Configure the level of Windows telemetry being sent out of the device. |Y|Y|Y|
| | | | | |
| [Windows Update](windows-update-management.md)                  | | | | |
|                        | Configure 'how' updates are installed    | |Y|Y**|
|                        | Configure 'what' updates are installed   | |Y|Y|

  '*' xperf.exe needs to be copied/installed to c:\windows\system32 on the IoT Enterprise sku.

  '**' 'ring' is not supported on the IoT Enterprise sku.

## Patterns

### IoT Hub Accessible Functionality

Everything listed in the feature table is accessible remotely through Azure IoT Hub.

### Device Accessible Functionality

While everything that is available through Azure IoT Hub can be accessed from the Azure DM UWP application (on the device) by forming the right json and sending it down to the DM library, we have also offered .Net APIs to facilitate some of the scenarios based on customer requests.

Below are the features/sub-features where such APIs exist and can be readily called from the UWP application running on the device.

- Factory Reset
  - Initiate factory reset
- Reboot Management
  - Immediate Reboots
  - Block system reboots.
- Time Settings
  - Time Service Control
- Windows Telemetry
  - Set the telemetry level.
- Windows Update
  - Retrieve the update status*.

**Note:** All items in the above list require connectivity to Azure IoT Hub except those marked with *.

### Policy

For settings that are controlled by both Azure IoT Hub Device Twin and by the local application, a policy need to be specified to give priority to one side or the other when both are set.
The following APIs make use of a policy:

- Time Service
  - Configuration
- Time Settings
  - Time Service Control

### Details Level

Given that the device twin has a limited capacity, some sections in the json document can be turned on or off, or its detail level changed to conserve the size.
The following areas provide the ability to change the detail level:

- Application Configuration
- Diagnostic Logs Configuration
- Wifi Settings
- Windows Updates
  
### Error Reporting

All operations will either report or return a [Status Object](status-reporting.md). Observers of the device twin can search/inspect such objects to know the time of the last desired state processed and the result.

## Infrastructure Features

### Connectivity Restoration

We have updated our library and samples applications to catch connection exceptions thrown by the Azure SDK, and attempt to re-construct all involved objects and re-connect.
See `ResetConnectionAsync` in either [Toaster @ MainPage.xaml.cs](../samples/ToasterApp/MainPage.xaml.cs) or [IoTDMBackground @ DMClientBackgroundApplication.cs](../samples/IoTDMBackground/DMClientBackgroundApplication.cs).

### Diagnosis

We have added a way to get an event trace of the Windows IoT Azure DM Client stack. This works for both retail and debug builds.
Details can be found on this [page](debugging.md).

### Inter-Process Security

We have re-enforced the security of the communication channel between the UWP application and the NT service. Details can be found on this [page](oem-device-setup.md) under *Configuring the Binaries*.

----

[Home Page](../README.md)
