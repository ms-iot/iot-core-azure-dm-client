# DM Hello World Application
## Deployment and Running

After completing the [Coding](dm-hello-world-coding.md) section, we now can start deployment.

The DM application depends on two other components that need to be on the device. Both components are built when the library is built (see [Building the Device Management Binaries](building-the-dm-binaries.md)).
In a production scenario, they will be part of the OS image (see [Production Device Setup](production-device-setup.md)). For our development scenario, however, we can install them manually as follows:

- **SystemConfigurator.exe**
  - Copy SystemConfigurator.exe to the device system folder `c:\windows\system32`.
  - Register the service by running:
      - `c:\windows\system32\SystemConfigurator.exe -install`
  - Start the service:
      - `net start SystemConfigurator`

- **DMHelloWorld**
  - Configure the remote machine in the application and hit F5.

**Next Step**:

- Managing the [Managing the Device](dm-hello-world-managing.md).

----

[Home Page](../README.md) | [DM Hello World Application](dm-hello-world-overview.md)