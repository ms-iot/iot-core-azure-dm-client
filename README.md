# Managing Windows IoT Devices through Azure IoT Hub

## Overview

**Windows IoT Azure DM Client Library** is an SDK that allows developers to build cloud solutions for managing Windows IoT Core devices with Azure IoT Hub.

The library is to be linked to a UWP application to allow both the Device Management components and the UWP application to share the connection to the Azure IoT Hub.

The application can be a foreground application or a background application. You can find [samples](docs/samples.md) for both and a [walk-through](docs/dm-hello-world-overview.md) to build a foreground application.

Here's a diagram of where the library fits:

<img src="docs/dm-architecture-all.png"/>

All device management operations are implemented via the Azure IoT Hub [direct methods](<https://docs.microsoft.com/en-us/azure/iot-hub/iot-hub-devguide-direct-methods>) and the [device twin](<https://docs.microsoft.com/en-us/azure/iot-hub/iot-hub-devguide-device-twins>).

## Quick Links

- [Building the Library](docs/building-the-library.md)
- [Walkthrough: DM Hello World Application](docs/dm-hello-world-overview.md)
- [Samples](docs/samples.md)
- [OEM Device Setup](docs/oem-device-setup.md)
- [Library Reference](docs/library-reference.md)
