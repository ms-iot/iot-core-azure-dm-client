# Running End-To-End Tests

The end-to-end tests uses a UWP application that mocks all the cloud pieces (device twin, etc) and uses the IoTDMClientLib.

Below are the steps necessary to build and run the tests:

- Build the solution for all three architectures.
- Deploy systemconfigurator.exe to the device and register/start the service.
- Create a nuget using the flavor you have built (i.e. debug or release).
- Add the nuget to the E2ETestApp.
- If you are running scenarios that depend on Azure storage, under E2ETestApp/TestInfrastructure/TestParameters.cs, fill in the parameters necessary (like Azure storage connection string, etc). Those scenarios are:
  - Application install scenarios.
  - Certificate install scenarios.
  - WiFi profile install scenarios.
- Build the E2ETestApp.
- Deploy E2ETestApp using Visual Studio (F5).
- Select the tests you want to run and click Execute.

To get detailed logs,

- You can either look at the debug output if you are running E2ETestApp under the debugger.
- You can collect the ETW logs as you would normally do for IoT Core Azure DM client ([see this page](debugging.md)).

----

[Home Page](../README.md) | [Library Reference](library-reference.md)