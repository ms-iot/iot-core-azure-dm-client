# Source Policy

For some device management areas, the desired state of a given property can be set:

- *Remotely* through the Device Twin.
- *Locally* through a .Net API interface. This interface can be called by the hosting UWP application.

Both, the remote configuration and the local configuration, can co-exist on the same device - however, only one of them can be applied. To specify which one, a priority has to be set. The priority can be set through either interface.

For example, consider the following:

- The administrator wants all devices to have the Time Service running.
- The administrator wants to give the option to the application user (on the device) to turn off the Time Service.
- The application exposes a way to turn on and off the Time Service (through its UI, for example).

For the above scenario, 

- The administrator will set the device twin properties to have the Time Service started and set the priority to `local`.
- The application can then call the .Net API (see below), and start/stop the service.

Should the administrator decide to take over, and override the application settings, the administrator can set the priority to `remote`, and then apply the desired settings.

Note that both the device twin and the .Net API can control the priority property - which gives them equal rights.

When reporting, it is the current state of the service that is reported - regardless of whether it is configured using the local or the remote settings.

----

[Home Page](../README.md) | [Library Reference](library-reference.md)