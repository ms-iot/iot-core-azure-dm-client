@echo off
@REM Examples:
@REM   PackIoTDMClientLib.cmd 1.4.4 Release
@REM   PackIoTDMClientLib.cmd 1.4.4 Debug

@if "%1"=="" goto MissingParameters
@if "%2"=="" goto MissingParameters

NuGet.exe pack "IoTDMClientLib.nuspec" -Prop Version=%1 -Prop Flavor=%2
goto End

:MissingParameters
@echo.
@echo Usage:
@echo     PackIoTDMClientLib.cmd version flavor
@echo.
@echo where:
@echo     version: the version of the nuget package to be genered in the form: major.minor.revision.
@echo     flavor : Debug or Release
@echo.
@echo Example:
@echo     PackIoTDMClientLib.cmd 1.4.4 Release
@echo.

:End
