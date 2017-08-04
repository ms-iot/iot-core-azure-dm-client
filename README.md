# Managing Windows IoT Devices through Azure IoT Hub

## Overview

[Azure IoT Hub](https://azure.microsoft.com/en-us/services/iot-hub/) and [Azure Device Management](https://docs.microsoft.com/en-us/azure/iot-hub/iot-hub-device-management-overview) offer a very scalable and efficient cloud service for managing millions of devices.

The Windows IoT team has implemented a device client (*Windows IoT Azure DM Client*) which communicate with those services and exposes a large number of the Windows management capabilities.

The *Windows IoT Azure DM Client* integrates tightly with the user's application on the device allowing certain interactions between them and also sharing the Azure IoT Hub connection (i.e. sharing the device identity).

The DM client consists of a UWP library (*Windows IoT Azure DM Client Library*) that is linked to the user's application, and a NT service (*SystemConfigurator*) and a proxy to connect the two (*CommProxy*).

The application can be a foreground application or a background application.

## Getting Started...

- [IoT Core Azure Device Management Overview](https://blogs.windows.com/buildingapps/2017/04/07/managing-windows-iot-core-devices-azure-iot-hub/)
- Azure IoT Hub
  - [Device Management Overview](https://docs.microsoft.com/en-us/azure/iot-hub/iot-hub-device-management-overview).
  - [Creating IoT Hub](https://docs.microsoft.com/en-us/azure/iot-hub/iot-hub-csharp-csharp-getstarted).
- [Samples Applications](docs/samples.md).
- [Application Creation Walk-through](docs/dm-hello-world-overview.md).

## Reference

- [Architecture](docs/dm-client-architecture.md)
- [Building the Library](docs/building-the-library.md)
- [OEM Device Setup](docs/oem-device-setup.md)
- [Library Reference](docs/library-reference.md)



