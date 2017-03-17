# Managing Windows IoT Devices through Azure IoT Hub

## Overview

Windows IoT Core devices can be managed via Azure IoT Hub. Azure IoT Hub can notify an application running on the device of configuration changes, commands issued, or simply hold status reported back from the device.

This library exposes many Windows functionalities by defining and implementing an interface which can be used by a management application talking to Azure IoT Hub.

The library is linked to a UWP application to allow both the library and the UWP application to share the connection to the Azure IoT Hub.

The application can be a foreground application or a background application. You can find a [samples](samples.md) for both and a [walk-through](walk-through.md) to build a foreground application.



All device management operation are implemented via the Azure IoT Hub [direct methods](<https://docs.microsoft.com/en-us/azure/iot-hub/iot-hub-devguide-direct-methods>) and the [device twin](<https://docs.microsoft.com/en-us/azure/iot-hub/iot-hub-devguide-device-twins>).

## Quick Links

- [Building the Library](building-the-library.md)
- [Walkthrough: DM Hello World Application](dm-hello-world-overview.md)
- [Samples](samples.md)
- [OEM Device Setup](oem-device-setup.md)
- [Library Reference](library-reference.md)

