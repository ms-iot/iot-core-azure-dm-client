
# dm-proto-private

To build:

### (1) Create a new folder and make it the current folder:
    * md d:\Projects\AzureDM
    * cd d:\Projects\AzureDM

### (2) Enlist in the Azure SDK
    * git clone --recursive --branch develop https://github.com/Azure/azure-iot-dt-sdks.git

### (3) Build the Azure SDK for x86
    * pushd.
    * cd azure-iot-dt-sdks\
    * md cmake-x86
    * cd cmake-x86
    * cmake -A x86 d:\Projects\AzureDM\azure-iot-dt-sdks\c
    * Open d:\Projects\AzureDM\azure-iot-dt-sdks\cmake-x86\azure_iot_sdks.sln
    * Build:
        * aziotsharedutil
        * iothub_client
        * iothub_client_mqtt_transport
        * umqtt

### (4) Build the Azure SDK for Other Architectures

(Optional) Repeat step 3, using `x64` or `arm` architectures instead of `x86`.

### (5) Set Environment Variable AZURE_IOT_DT_SDK

Environment variable AZURE_IOT_DT_SDK points to the location of your Azure SDK repo, for example:

    set AZURE_IOT_DT_SDK=d:\Projects\AzureDM\azure-iot-dt-sdks
    
or, in Powershell:

    $env:AZURE_IOT_DT_SDK="d:\Projects\AzureDM\azure-iot-dt-sdks"

This is needed for DM client build.

### (5) Enlist in the DM client
    * git clone --recursive --branch master https://github.com/ms-iot/dm-proto-private.git

### (6) Open d:\Projects\AzureDM\dm-proto-private\IoTDMAgent\IoTDMAgent.sln
    * Build.

### (7) Install and start the serivce
  - Copy the following binaries to the device under any folder (c:\temp).
    * IoTDMAgent.exe
    * concrt140d.dll
    * cpprest140d_2_8.dll
    * LimpetLib.dll
    * msvcp140d.dll
    * wastorage.dll
  - c:\temp\IoTDMAgent.exe -install
  - net start IoTDMAgent
