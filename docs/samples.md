# Samples

Microsoft has created a sample implementation for the application on the device as well as the operators application. You can find these samples on in ms-iot/iot-core-azure-dm-client/samples. This will provide the foundation to build fully functional Device Management into your device.

Here is a high level description of what is there:

1. **Toaster** - a foreground UI *Device Application* representing a fictional smart toaster.  In this case, device management is a part of the *Device Application*.  This is a UWP app that can be deployed from Visual Studio (using either Build > Deploy Solution or F5).
1. **IoTDMBackground** - a Background Application enabling simple device managment independent of any UI app.  In this case the *Device Application* is totally separated from the device management.  This is a UWP Background Application that can be deployed from Visual Studio (using either Build > Deploy Solution or F5).
1. **DMDashboard** - a *Device Management Dashboard* allowing registered smart toasters to be managed.  This is a .NET desktop application that can be built using Visual Studio.

## Running the Samples

To leverage these items on your IotCore device, follow these steps (steps below assume Toaster, but will work if applied to IoTDMBackground as well):

1. Setup your [Azure IotHub](https://ms.portal.azure.com/) account.
1. Install [Visual Studio 2017 Update 2](https://www.visualstudio.com/downloads) or later (be sure UWP features are enabled).
1. Setup your IotCore device using [IoT Dashboard](https://docs.microsoft.com/en-us/windows/iot-core/connect-your-device/iotdashboard).
1. Download the [iot-core-azure-dm-client code](https://github.com/ms-iot/iot-core-azure-dm-client) from github and open **IoTDM.sln** in Visual Studio.
    + Ensure that the Toaster application can connect to IoTHub:
      + You can hardcode the IoTHub device string in the code (see iot-core-azure-dm-client\samples\ToasterApp\MainPage.xaml.cs @ GetConnectionStringAsync()).
      + Or, you can store the connection string into the TPM (using the web portal, for example) and let the Toaster application retrieve it from there and use it to connect.
1. Configure the Toaster project as the StartUp project by selecting Toaster in the Solution Explorer and selecting **Set as StartUp Project** in the Project  menu (this option is also available in the Solution Explorer's right click context menu).
1. Set the target configuration/architecture as required by your device and **Build > Rebuild Solution**
1. Deploy to your device by selecting Remote Machine, entering your device's IP address (or name), and **Build > Deploy Solution**.
1. Using SSH (or PowerShell) connect to your device as Administrator.
1. Verify that **SystemConfigurator.exe** is present:

        Toaster with Debug ARM configuration: C:\Data\Users\DefaultAccount\AppData\Local\DevelopmentFiles\ToasterUiVS.Debug_ARM.msft
        IoTDMBackground with Debug ARM configuration: C:\Data\Users\DefaultAccount\AppData\Local\DevelopmentFiles\23983CETAthensQuality.IoTDMBackgroundSampleVS.Debug_ARM.msft

1. From **SSH** shell, run (assuming Toaster app with Debug ARM configuration, for IoTDMBackground use path specified in previous step):

        cd C:\Data\Users\DefaultAccount\AppData\Local\DevelopmentFiles\ToasterUiVS.Debug_ARM.msft
        net stop SystemConfigurator
        SystemConfigurator.exe -remove
        SystemConfigurator.exe -install
        net start SystemConfigurator

1. From **Visual Studio**, use F5 to launch Toaster app.  This should bring the **Toaster** app to your device and start it.
1. Run **DMDashboard** on your desktop by launching DMDashboard.exe
    + DMDashboard can trigger reboot in device, get and change time info, get and change device status, and install an App by using Azure blob storage.

For production environments, see [OEM Device Setup](oem-device-setup.md).

----

[Home Page](../README.md)