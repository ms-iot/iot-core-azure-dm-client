
# dm-proto-private

To build:

### (1) Create a new folder and make it the current folder:
    * md d:\Projects\AzureDM
    * cd d:\Projects\AzureDM

### (2) Enlist in the project
    * git clone --recursive --branch develop https://github.com/ms-iot/dm-proto-private.git

### (3) Build the solution
    * cd src
    * nuget restore
    * msbuild IoTDM.sln

To run the Toaster sample:

### (1) Start SystemConfigurator:
   * TBD
   
### (2) Run Toaster sample from VS:
   * Right-click on the Toaster project and select Deploy
   * Copy src\Debug\CommProxy.exe into the Appx directory of the Toaster app
   * Right-click on the Toaster project and select Debug
   (the first two steps are one-time-only)
