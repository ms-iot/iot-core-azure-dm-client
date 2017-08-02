# Building The Library

To build the library:

#### Enlist

  - Create a new folder, let's say `c:\iot-core-azure-dm-client`. and make it the current folder.
  - Run `git clone --recursive --branch master https://github.com/ms-iot/iot-core-azure-dm-client`

#### Build The Binaries

  - Open `src\IoTDM.sln` in Visual Studio.
  - Set the configuration to *Release*.
  - Build the solution for each of the three architectures (ARM, x64, and x86).

#### Build The Nuget Package

  - Open a Visual Studio command prompt and 
    - Make sure you have Nuget.exe on the path.
      - If not, or if it is not present, download version v3.5.0 from this [Nuget.org](<https://dist.nuget.org/index.html>).
    - `cd c:\iot-core-azure-dm-client\nuget`
    - Run `PackIoTDMClientLib.cmd`

This will create `c:\iot-core-azure-dm-client\nuget\IoTDMClientLib.1.0.0.nupkg`.

**Next Step**:

- See the [DM Hello World! Application](dm-hello-world-overview.md).