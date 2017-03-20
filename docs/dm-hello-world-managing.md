# DM Hello World Application
## Managing the Device

Note that we have deployed the DMHelloWorld application and the service is running, we can start testing some of the DM features.

In order to do that, we need to either do direct method calls or configure the Device Twin desired property. Both can be done through the [Microsoft.Azure.Devices SDK](https://www.nuget.org/packages/Microsoft.Azure.Devices/1.1.0).

There are two samples that can help here:

- [Device Explorer](https://github.com/Azure/azure-iot-sdk-csharp/tree/master/tools/DeviceExplorer)
  - This tools allows the users to experiment by constructing their own json by hand and merging it into the Device Twin. It also allows the users to type in the direct method names and json parameters and sends them to the device.

- DMDashboard
  - This is a sample that is distributed with the Windows DM client library solution. This sample demonestrates how to construct the json that the DM client library understands. It also exposes all the supported functionality through easy-to-use UI.

Once the DMHelloWorld is up and running, launch the DMDashboard, connect to your IoT Hub account and device, and start experimenting by reading the current state of the device, or configuring it.