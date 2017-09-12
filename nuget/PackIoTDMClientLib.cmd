@REM Examples:
@REM   PackIoTDMClientLib.cmd 1.4.4 Release
@REM   PackIoTDMClientLib.cmd 1.4.4 Debug

NuGet.exe pack "IoTDMClientLib.nuspec" -Prop Version=%1 -Prop Flavor=%2
