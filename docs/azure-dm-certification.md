# Azure Device Management Certification

The Azure Device Management Certification process defines a minimum set of features the device management client has to comply with in order to be certified.

In order to verify those features, follow these steps:

1. Create an IoT Hub and a new device in that hub.

2. Build a device such that the Azure device management client is up and running and is connected to the IoT Hub with the identity of the device you have created.
  - You may choose to build your own client or use the client implemented in this repro as-is or modified (see more information [here](oem-device-setup.md)).

3. On a desktop machine, run the verification application (DMValidator) by pointing it to the IoT Hub and the device you have created in step 1.
  - The verification application can be downloaded from [here](https://github.com/ms-iot/iot-core-azure-dm-client/tree/master/tests/DMValidator).
  - Select all the test scenarios and then click 'Run Test Cases'.

4. Collect and compress the logs DMValidator generates after running the test scenarios.

5. Send the compressed logs to Microsoft Azure device management certification team.

----

[Home Page](../README.md)