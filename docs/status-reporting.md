# Status Reporting

Status reporting takes place through several channels:

- IoT Hub
  - When applying desired property changes or executing direct methods, the DM Client will update the device twin with a <i>Status Object</i> (see below) under the section that corresponds to the area being updated.
- ETW Logs
  - For scenarios where there an incorrect behavior and more data needs to be gathered, ETW can be enable for the problematic area and logs can be collected. For more details, see [Diagnostic Log Management](diagnostic-logs-management.md) and [Debugging Azure Device Management Client](debugging.md).
- .NET Library
  - <i>Callbacks: ToDo</i>
  - <i>Exceptions: ToDo</i>

### The Status Object

The <i>Status Object</i> has the following properties:

<pre>
{
    "time" : "received time stamp",
    "state": "pending|committed|failed",
    "errSubSystem" : sub-system,
    "errCode" : error code,
    "errContext" : "error context"
}
</pre>

- `time`: The time stamp of when the request is received.
- `state`: Possible values are:
  - `pending`: the request has been received, and is being processed.
  - `committed`: the request has been processed successfully.
  - `failed`: the request has failed. The error fields should be set with details.
- `errSubSystem` : the source of the error. This helps decode the error number returned.
  - `DeviceManagement`: error originated in the Azure device management stack.
  - `Win32`: error originated from a win32 api.
  - `Unknown`: an unknown compenent threw an exception.
- `errCode`: the error code returned when the operation failed. This can be decoded by looking up the errors defined by the reported subsystem.
- `errContext`: this will indicate information specific to the failure - for example, if a file fails to open, it will contain the name of the file.

----

[Home Page](../README.md) | [Library Reference](library-reference.md)