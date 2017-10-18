/*
Copyright 2017 Microsoft
Permission is hereby granted, free of charge, to any person obtaining a copy of this software
and associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH
THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#pragma once

#define IoTDMRegistryRoot L"Software\\Microsoft\\IoTDM"

#define ValueUnspecified L"<unspecified>"

#define RegTrue L"True"
#define RegFalse L"False"
#define RegReportToDeviceTwin L"ReportToDeviceTwin"
#define RegPolicySource L"PolicySource"
#define RegSourcePriorities L"SourcePriorities"
#define RegPolicySeparator L';'

#define RegLastRebootCmd L"LastRebootCmd"

#define RegWindowsUpdateRebootAllowed L"WindowsUpdateRebootAllowed"
#define RegWindowsUpdatePolicySectionReporting L"WindowsUpdatePolicySectionReporting"

#define RegEventTracing IoTDMRegistryRoot L"\\EventTracingConfiguration"
#define RegEventTracingLogFileFolder L"LogFileFolder"
#define RegEventTracingLogFileName L"LogFileName"

#define RegTimeService IoTDMRegistryRoot L"\\TimeService"
#define RegRemoteTimeService RegTimeService L"\\Remote"
#define RegLocalTimeService RegTimeService L"\\Local"

#define RegTimeServiceEnabled L"Enabled"
#define RegTimeServiceStartup L"Startup"
#define RegTimeServiceStarted L"Started"

#define RegTimeServiceAuto L"auto"
#define RegTimeServiceManual L"manual"

#define RegWindowsUpdatePolicy IoTDMRegistryRoot L"\\WindowsUpdatePolicy"
#define RegRemoteWindowsUpdatePolicy RegWindowsUpdatePolicy L"\\Remote"
#define RegLocalWindowsUpdatePolicy RegWindowsUpdatePolicy L"\\Local"

#define RegWindowsUpdateActiveHoursStart L"ActiveHoursStart"
#define RegWindowsUpdateActiveHoursEnd L"ActiveHoursEnd"
#define RegWindowsUpdateAllowAutoUpdate L"AllowAutoUpdate"
#define RegWindowsUpdateAllowUpdateService L"AllowUpdateService"
#define RegWindowsUpdateBranchReadinessLevel L"BranchReadinessLevel"
#define RegWindowsUpdateDeferFeatureUpdatesPeriod L"DeferFeatureUpdatesPeriod"
#define RegWindowsUpdateDeferQualityUpdatesPeriod L"DeferQualityUpdatesPeriod"
#define RegWindowsUpdatePauseFeatureUpdates L"PauseFeatureUpdates"
#define RegWindowsUpdatePauseQualityUpdates L"PauseQualityUpdates"
#define RegWindowsUpdateScheduledInstallDay L"ScheduledInstallDay"
#define RegWindowsUpdateScheduledInstallTime L"ScheduledInstallTime"
#define RegWindowsUpdatePolicyRing L"Ring"

#define RegWindowsUpdatePolicyEarlyAdopter L"EarlyAdopter"
#define RegWindowsUpdatePolicyPreview L"Preview"
#define RegWindowsUpdatePolicyGeneralAvailability L"GeneralAvailability"
