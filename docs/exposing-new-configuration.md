# Exposing New Configuration

## Overview

The Azure DM client solution here consists of two main components:

- The Azure DM client library.
    - This library provides an object (`DeviceManagementClient`) that can do the following:
        - Share the Azure IoT Hub connection with the UWP hosting application.
        - Provide handlers for notifications coming from the IoT Hub (desired property changes, or direct methods).
        - The handlers parse the requests, then apply some business logic if necessary, and then pass them on to the SystemConfigurator NT service.
- The SystemConfigurator NT service.
    - This service runs as a local system and hence it is capable of executing functions that require elevated privileges and is also capable of accessing the system CSPs.
    - The service receives requests from the Azure DM client, parses them, and then routes the request to its handler.
    - The handler constructs SyncML with the passed in parameters and sends it down to the CSP. If the functionality is not supported through a CSP, the handler can call Win32 apis as well.

## High-level Walk Through

So, in order to expose new configuration, the work involved is:

- Design the device twin schema.
    - Decide the desired and reported properties and their hierarchy.
- Determine wheather the functionality can be done in the UWP app or it has to be done in the elevated NT service (SystemConfigurator).
    - Implementing changes in the SystemConfigurator is more expensive since the requests have to be routed all the way down and back, and then route the responses back up.
- If the functionality can be done in UWP, determine whether it needs to be part of the Azure DM client library or part of the user code (your code).
    - If the functionality is generic enough, it would be beneficial for the community to add it to the Azure DM client library. It is best to loop us in as early as possible so we can decide whether it is something we can take in or not.
- If the functionality requires elevated privileges, or the API is not available in UWP, then your only option is to implement it in the elevated NT service.

The easiest way is to look at one of the already implemented areas and see how it is done. The Reboot Info is a good start...

## Extending the Azure DM Client Library

- Design the device twin schema (desired and reported properties).
- Implement a helper data serialization class in the DMDataContract (see [src\\DMDataContract\\DMDataContract\\RebootInfo.cs](..\src\DMDataContract\DMDataContract\RebootInfo.cs)).
    - This is option, but using this pattern proved to be convenient especially when writing the portal side or writing tests.
- Implement a handler (see [src\\IoTDMClientLib\\Handlers\\RebootInfoHandler.cs](..\src\IoTDMClientLib\Handlers\RebootInfoHandler.cs)).
- Instantiate the handler and connect it so it is called when the corresponding property changes in the device twin (see [src\\IoTDMClientLib\\DeviceManagementClient.cs](..\src\IoTDMClientLib\DeviceManagementClient.cs) @ CreateAsync()).

### Debugging

The easiest way to debug changes to the Azure DM Client library is to modify the one in the included solution and then use the Toaster application to debug the library. The solution is already configured so that you can just F5. Of course, you need to make sure the Toaster app has the IoT Hub device connection string so you can communicate with it remotely.

## Extending SystemConfigurator

In order to forward the request from the Azure DM Client library down to the SystemConfigurator, 
- The Azure DM Client needs to package the request and send it...
- And then, the SystemConfigurator needs to unpackage the request and act on it...
- Finally, the SystemConfigurator needs to package a response and send it back to the Azure DM Client... which in turn unpackages and communicates to the device twin.

The packaging and unpackaging of the data flowing between the Azure DM client and the SystemConfigurator is done by the DMMessage WinRT component.
This component provides a way to serialize/deserialize the data, and is also callable from both the Azure DM client library and the SystemConfigurator.

So, the steps to add the new functionality are:

- Design what data needs to be sent from the UWP client library to the SystemConfigurator and what the response should look like.
- Implement a DMMessage serialization class (see [src\\DMMessage\\Models\\Reboot.h](..\src\DMMessage\Models\Reboot.h)).
- Create a routing for the requests by:
    - Add a new entry in [src\\DMMessage\\Models\\ModelsInfo.dat](..\src\DMMessage\Models\ModelsInfo.dat).
        - See: `MODEL_ALLDEF(GetRebootInfo, 17, GetRebootInfoRequest, GetRebootInfoResponse )`
            - `GetRebootInfo`: this is the name of the request. It will be used to deduce the name of the handler in the code (HandleGetRebootInfo @ CommandProcess.cpp).
            - `GetRebootInfoRequest`: name of the serialization class used for the request from the Azure DM client library to SystemConfigurator.
            - 17: a unique code to identify the request.
            - `GetRebootInfoResponse`: name of the serialization class used for the response from SystemConfigurator to the Azure DM client library.

    - Implement a handler in [src\\SystemConfigurator\\CommandProcessor.cpp](..\src\SystemConfigurator\CommandProcessor.cpp).
        - The handler name must be on the form: "`Handle` + `first parameters in the MODEL_ALLDEF`" above.
        - Once you have the handler in SystemConfigurator, you can read the parameters from the request object, act on it, and then construct a response object and return it.
        - See [HandleRebootInfo()](..\src\SystemConfigurator\CommandProcessor.cpp) in [src\\SystemConfigurator\\CommandProcessor.cpp](..\src\SystemConfigurator\CommandProcessor.cpp).

- If your handler needs to access a CSP, you can use one of the `MdmProvision::` helpers defined in [src\\SystemConfigurator\\CSPs\\MdmProvision.h](..\src\SystemConfigurator\CSPs\MdmProvision.h)

### Debugging

SystemConfigurator does not do anything when it starts - it just waits for incoming requests from the Azure DM client library. This simplifies debugging significantly as we do not have to deal with start-up issues.

- On the device:
    - Make sure SystemConfigurator is up and running.
    - From a PowerShell window, run tlist and note the process id of the SystemConfigurator:
    - Start the debugger server: `mwdbgsrv.exe -t tcp:port=5000,IcfEnable`.
- On the developer's box:
    - Start WinDBG: `c:\debuggers\windbg -premote tcp:port=5000,server=machineip -p processid_from_tlist`
- Set your symbols paths to where you build SystemConfigurator
- Start your UWP application (i.e. Toaster) so that you can modify the device twin, and see the changes coming down through the Azure DM library and then into SystemConfigurator.
- Modify the device twin to exercise the scenarios you want to test...

----

[Home Page](../README.md)