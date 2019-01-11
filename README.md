# Managing Windows IoT Devices through Azure IoT Hub

## Announcement - January 10th, 2019

We have released a new version of the Windows Azure DM Client at [https://github.com/ms-iot/azure-client-tools](https://github.com/ms-iot/azure-client-tools). It will eventually replace the code in this repo. Check-out the new version, and let us know what you think!

## Overview

Device Management (DM) allows operators to remotely configure and monitor very high volumes of IoT devices simultaneously.

Configuration for the devices can be pushed to devices, whether the target devices are online or offline.  If the device is offline it will pick up the new configuration when it reconnects.  Device operators can also retrieve the status of each device, including whether device configuration updates have been successfully applied or not.

Enabling these features for IoT devices requires a central, robust, and reliable mechanism to store and to deploy the configuration data to the target devices, and monitor device status - at scale.

A complete solution requires the following:

- **A Robust/Scalable Cloud Service** to store the desired and reported states of the various devices.
  - [Azure IoT Hub](https://azure.microsoft.com/en-us/services/iot-hub/) and [Azure Device Management](https://docs.microsoft.com/en-us/azure/iot-hub/iot-hub-device-management-overview) offer a highly  scalable and efficient cloud service for managing millions of devices.

- **A Client** that runs on the device and can apply the desired configuration and report the state of the device.
  - The ***Windows IoT Azure DM Client*** (an open source SDK + runtime), in conjunction with Azure IoT Hub, can apply and report on a large set of common, sometimes critical, Windows configurations.

- **A Portal or an Application** that will be used by the operator to configure and query the devices remotely.
  - This must be customized for devices by the device OEM or operator.  As part of this solution, we also provide an open source data model and implementation for easier interaction with the IoT Hub storage and the Windows IoT client.

This repo holds the ***Windows IoT Azure DM Client*** open source SDK + runtime components necessary to achieve the described scenario.

## Windows IoT Azure DM Client

### Release Notes

### [October 2017 - Full Feature List](docs/release-notes-2017-10.md)

### [June 2018 - Full Feature List](docs/release-notes-2018-06.md)

### Overview

The *Windows IoT Azure DM Client* integrates tightly with the user's application on the device. This integration allows some interaction scenarios between the remote configuration and the application business logic.

The DM client consists of:

- A UWP library (*Windows IoT Azure DM Client Library*) that is to be linked to the user's application. The application can be a foreground application or a background application.
- An NT service (*SystemConfigurator*) that is running with System privilege and can listen to Azure IoT Hub notifications relayed by the UWP library and carry out the necessary device management actions.

## Getting Started...

- [IoT Core Azure Device Management Overview](https://blogs.windows.com/buildingapps/2017/04/07/managing-windows-iot-core-devices-azure-iot-hub/)
- Azure IoT Hub
  - [Device Management Overview](https://docs.microsoft.com/en-us/azure/iot-hub/iot-hub-device-management-overview)
  - [Creating IoT Hub](https://docs.microsoft.com/en-us/azure/iot-hub/iot-hub-csharp-csharp-getstarted)
- [Sample Applications](docs/samples.md)
- [DM Application Creation Walk-through](docs/dm-hello-world-overview.md)
- [Exposing New Configuration Walk-through](docs/exposing-new-configuration.md)

### System Requirements

#### Developer's Box

- Visual Studio 2017 ([download](https://www.visualstudio.com/downloads)).
  - Make sure the following is selected:
    - VC++ 2017 v141 toolset (x86, x64)
    - Visual C++ compilers and libraries for ARM.
    - Visual C++ 2017 redistributable update.
    - Visual C++ runtime for UWP.
    - Visual Studio C++ Core Features
    - Visual C++ ATL support
    - Windows 10 SDK (10.0.15063.0) for Desktop
    - Windows 10 SDK (10.0.15063.0) for UWP
    - Windows 10 SDK (10.0.16299.0) for UWP  (*For DMController*)


#### Device

- Windows IoT Core build 15063 (March 2017) or later.

## Reference

- [Architecture](docs/dm-client-architecture.md)
- [Building the Device Management Binaries](docs/building-the-dm-binaries.md)
- [OEM Device Setup](docs/oem-device-setup.md)
- [Library Reference](docs/library-reference.md)
- [Device Provisioning Service(DPS) Client](<https://github.com/ms-iot/iot-azure-dps-client>)
- [Azure Device Management Certification](docs/azure-dm-certification.md)
