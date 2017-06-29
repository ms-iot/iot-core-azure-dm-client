# Diagnostic Logs Management

## Overview

Event Tracing for Windows (ETW) is a mechanism provided on the Windows platform that allows processes to log messages with minimal overhead.

Part of how ETW achieves that is by moving all the logic of managing which messages go into log files, how the log files are named, their size limits, etc out of the running process.

This decoupling is done by the introduction of the concept of a 'provider' (i.e. the process that is writing the log message) and a 'collector' (i.e. the process that is reading the log messages) - and a 'controller' (the OS) in between to connect the 'providers' and 'collectors'. The relationship between collectors and providers are many-to-many.

Both providers and collectors are identified on the system using ids (assigned at creation time). The user can then specify which provider a given collector can listen to using those ids.

Collectors have properties that describe various things; for example: how the messages should be saved to a file, what is the maximum size of the file, etc. For each provider, a collector can define some properties; like log level for example (i.e. critical, error, information, etc).

The logical hierarchy is as follows:

<pre>
- Collector1
  - LogFileSizeLimitMB = 4
  - LogFileFolder = "AzureDM"
  - Provider1
    - TraceLevel = critical
  - Provider2
    - TraceLevel = error
- Collector2
  - ...
</pre>

Above, `Collector1` is defined to listen to two providers; `Provider1` and `Provider2`. `Collector1` will write only critical messages from `Provider1` and error (or higher priority) messages from `Provider2`.

Here is a link to the [Event Tracing architecture on MSDN](https://msdn.microsoft.com/en-us/library/windows/desktop/aa363668.aspx).

## When to Use Azure DM Diagnostic Logs Management

A typical usage scenario is that there is a problem with a certain process running on the system.

- If that process does not log using ETW, then, this mechanism is not for it.
- If that process is using ETW, then it is a provider and it logs its message to the OS under a certain guid. The user needs to find out which guid it is.

Once we have the provider(s) guid(s), we can define a collector and list the providers underneath - along with all the necessary configurations for the collector and the providers.

## How to Use Azure DM Diagnostic Logs Management

Windows IoT Azure DM exposes ETW configuration by wiring-up the [Diagnostic Log CSP](https://docs.microsoft.com/en-us/windows/client-management/mdm/diagnosticlog-csp). The logic and properties described below is very closely based on how the CSP is designed.

Here are the steps to capture logs to a file on disk:

- Identify the provider(s) you want to capture (find out the guids you need to collect).
- Create a collector, configure it, and add the providers you want captured to it.
- Start collection.
- Stop collection. This saves a log file on disk in a pre-configured folder.

Here are the steps to upload a log file:

- Enumerate all the files in the specified log folders.
- Provide the source file name on disk, the target Azure Storage parameters (connection string, container), and the Azure DM client will upload it for you.

## Creating, Configuring, and Starting/Stopping Collectors

Collectors are created by simply defining them in the device twin <i>desired</i> properties section. Each collector exposes a set of properties that defines its operation along with a set of providers and how each of them is processed by the collector.

Below is what the schema looks like:

### Collectors List and DeviceTwin Interaction

<pre>
"Windows" : {
    "eventTracingCollectors" : {
        "collector00" : {
            "reportProperties" : "yes"|"no",
            "applyProperties" : {collector configuration object}|"no"
            },
        "collector01" : {
            "reportProperties" : "yes"|"no",
            "applyProperties" : {collector configuration object}|"no"
            },
        "?" : "detailed" | "minimal" | "none"
        }
    }
}
</pre>

- `"eventTracingCollectors"`: is the node where all collector configurations appear under.
    - `"collector00"`: is the name of the collector being created.
        - `"reportProperties"`: indicates whether the state of this collector should be reported to the device twin or not. Allowed values are `"yes"` or `"no"`.
        - `"applyProperties"`: indicates which properties are to apply on the device - if any. Allowed values are:
            - `"no"`: indicates no properties to apply from the device twin. This makes sense only if the collector is already defined on the device and the user does not want to modify it.
            - `{collector configuration object}`: defines the collector configurations to apply. See below for more details.
    - `"?"`: specifies what that the DM client should report about the existing collectors. This can be used to enumerate defined collectors.
      - `"detailed"`: report all details.
      - `"minimal"`: report only the names of the defined collectors.
      - `"none"`: don't report except those that are marked to be reported in the desired section.

### Collector Configuration Object

<pre>
{
    "traceLogFileMode" : "sequential"|"circular",
    "logFileSizeLimitMB" : <i>limit</i>,
    "logFileFolder" : "<i>collectorFolderName</i>",
    "started" : "yes" | "no",
    "guid00" : {provider configuration object},
    "guid01" : {provider configuration object}
}
</pre>

- `"traceLogFileMode"`: specifies the log file logging mode. Allowed values are `"sequential"` or `"circular"`. Note that when it is set to `"sequential"`, logging will stop after the file reaches out its set limit.
- `"logFileSizeLimitMB"`: specifies the limit for the log file in megabytes. The default value is 4, and the acceptable range is 1-2048.
- `"collectorFolderName"`: specifies the relative path to the user's data folder where the log files of that collector will be saved once collection stops. The folder name cannot include `\` (The files can later be enumerated and uploaded to Azure Storage. See below for more details).
- `"started"`: specifies whether the collector should be active (i.e. collecting) or not. Its value is applied everytime the DM client service starts, or the property changes.
  - If this is set to `"yes"`, the collector will be started (if it is not already).
  - If this is set to `"no"`, the collector will be stopped, and a file will be saved in <i>logFileFolder</i> (if it is already running).
- `"guid00"`: specifies the <i>provider configuration object</i> for this guid. See below for more details.

### Provider Configuration Objectwindows

<pre>
{
    "traceLevel": "critical"|"error"|"warning"|"information"|"verbose",
    "keywords": "<i>see below</i>",
    "enabled": true|false,
    "type": "provider"
}
</pre>

- `"traceLevel"`: specifies the level of detail included in the trace log. Allowed values are `"critical"`, `"error"`, `"warning"`, `"information"`, and `"verbose"`.
- `"keywords"`: specifies the provider keywords to be used as MatchAnyKeyword for this provider.
- `"enabled": specifies if this provider is enabled in the trace session. Allowed values are `true` or `false`.
- `"type"`: specifies that this object is a provider object for the DM client. The only allowed value is `provider`.

## Reporting

Collectors are reported if:

- They are defined in the desired section and their `"reportProperties"` is set to `"yes"`. For those, all details are always reported.
- The `"?"` is specified in the collectors list in the desired properties section. The level of details will depend on its value - `"detailed"` or `"minimal"`.

The reporting for `"detailed"` looks like this:

<pre>
"windows" : {
    "eventTracingLogs" : {
        "collectorName00" : {
            "traceLogFileMode" : "sequentual"|"circular"",
            "logFileSizeLimitMB" : "4",
            "logFileFolder" : "collectorFolderName",
            "started" : "yes"|"no",
            "guid00" : {
                "traceLevel": "critical"|"error"|"warning"|"information"|"verbose",
                "keywords" : "",
                "enabled" : true|false,
                "type" : "provider"
            }
        }
    }
}
</pre>

The reporting for `"minimal"` looks like this:
<pre>
"windows" : {
    "eventTracingLogs" : {
        "collectorName00" : "",
        "collectorName01" : "",
        "collectorName02" : ""
        }
    }
}
</pre>

## Working with Log Files

To upload collected log files, 

- The device must be online.
- Enumerate saved files by invoking the direct methods `windows.enumDMFolders` and `windows.enumDMFiles`.
- Upload a file (given its folder and name) by invoking the direct method `windows.uploadFile`.

To delete collected log files from the device,

- The device must be online.
- Enumerate saved files by invoking the direct methods `windows.enumDMFolders` and `windows.enumDMFiles`.
- Delete a file (given its folder name) by invoking the direct method `windows.deleteFile`.

### windows.enumDMFolders

Call this method to get a list of IoTDM subfolder names. The IoTDM folder is `C:\Data\Users\DefaultAccount\AppData\Local\Temp\IotDm`. The subfolder names are the names specified in collectors LogFileFolder properties.

#### Input
<i>none</i>

#### Output
<pre>
{
    "list" : [
        "folder0",
        "folder1",
        ...
        "folderN",
    ],
    "errorCode": <i>errorCode</i>,
    "errorMessage": <i>errorMessage</i>
}
</pre>

- `"errorCode"`: `windows.enumDMFolders` returns 0 if successful. Otherwise, it returns the error code.
- `"errorMessage"`: This will be empty if the method call suceeded. Otherwise, it will have the error message if available.

### windows.enumDMFiles

Call this method to get a list of the saved log files under the log specified log folder.

#### Input

<pre>
{
    "folder" : "<i>folderName</i>"
}
</pre>

Notes:

- `folderName` is the name of the folder under the IoTDM data folder.

For example:

<pre>
{
    "folder" : "AzureDM",
}
</pre>

#### Output

<pre>
{
    "list": [
        "AzureDM_2017_07_18_11_14_38.etl",
        ...,
        "AzureDM_2017_07_20_18_14_38.etl"
    ],
    "errorCode": <i>errorCode</i>,
    "errorMessage": <i>errorMessage</i>
}
</pre>

- `"errorCode"`: `windows.enumDMFiles` returns 0 if successful. Otherwise, it returns the error code.
- `"errorMessage"`: This will be empty if the method call suceeded. Otherwise, it will have the error message if available.
}
</pre>

### windows.uploadDMFile

Call this method to upload a saved file to Azure Storage.

#### Input

<pre>
{
    "folder" : "<i>folderName</i>",
    "fileName" : "<i>fileName</i>",
    "connectionString": "<i>connectionString</i>",
    "container": "<i>containerName</i>"
}
</pre>

Notes:

- `folderName` is the name of a folder under IoTDM folder.

#### Output

<pre>
{
    "errorCode": <i>errorCode</i>,
    "errorMessage": <i>errorMessage</i>
}
</pre>

- `"errorCode"`: `windows.uploadDMFile` returns 0 if successful. Otherwise, it returns the error code.
- `"errorMessage"`: This will be empty if the method call suceeded. Otherwise, it will have the error message if available.

Notes:

- The upload is asynchronous. So, the file should appear on Azure Storage sometime later after the method response is received.

  <i>ToDo: Need a better error reporting mechanism here.</i>

#### windows.deleteFile

Call this method to delete a saved file from the device storage.

#### Input

<pre>
{
    "folder" : "<i>folderName</i>",
    "fileName" : "<i>fileName</i>",
}
</pre>

Notes:

- `folderName` is the name of a folder under IoTDM folder.

#### Output

<pre>
{
    "errorCode": <i>errorCode</i>,
    "errorMessage": <i>errorMessage</i>
}
</pre>

- `"errorCode"`: `windows.enumDMFolders` returns 0 if successful. Otherwise, it returns the error code.
- `"errorMessage"`: This will be empty if the method call suceeded. Otherwise, it will have the error message if available.
