# Introduction 
DMController is an UWP app that will be used by the operator to configure and query the devices remotely. This must be customized for devices by the device OEM or operator. As part of this solution, we also provide an open source data model and implementation for easier interaction with the IoT Hub storage and the Windows IoT client.

## System Requirements
### Developer's Box

- Visual Studio 2017 ([download](https://www.visualstudio.com/downloads)) with the following:
	- Windows 10 SDK (10.0.16299.0) for UWP  (*For DMController*)

## Getting Started
### Basic controls

<img src="imgs/settings.png"/>
- When the application first launches, enter the **IoT Hub Connection String** in the Settings section to connect to the devices.
- **The Storage Connection String** is required for specific features such as installing an applications, certificates, wifi profiles, etc.
- Select a **Device** on the bottom left to configure and query the selected device remotely. 

<img src="imgs/deviceinfo-buttons.png"/>

- The **Get All Device Properties** button will reports all device properties supported by the Windows IoT Azure Device Client.
- The green refresh button will get the latest Device Information of the selected device.  

### Test scenarios
The two buttons beside the Settings button will perform the following test scenario:

<img src="imgs/test-buttons.png"/>
- Start an UWP in the foreground on all connected devices. 
- Stop an UWP in the foreground on all connected devices. 
- Change the test UWP Package Family name in DMController/MainPage.cs:

        const string DemoAppPackage = "DMClientOverview_jv0mnc6v5g12j";
