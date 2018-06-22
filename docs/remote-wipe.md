# Remote Wipe

A **Remote Wipe** operation wipes all the data from the system. It also allows the clearing of the TPM.

The **RemoteWipe** operation is initiated by either:

- The application calling `DeviceManagementClient.StartRemoteWipeAsync()`. 
- The application calling `DeviceManagement.StartRemoteWipeAsync()`. 
- The operator invoking the Azure direct method `windows.startRemoteWipeAsync`.

### DeviceManagementClient.StartRemoteWipeAsync()

<pre>
    <b>Namespace</b>:
	Microsoft.Devices.Management
</pre>

<pre>
    <b>Class</b>:
    DeviceManagementClient
</pre>

<pre>
    <b>Methods</b>:
    public async Task StartRemoteWipeAsync(bool clearTPM)
</pre>

Notes:

- `DeviceManagementClient.StartRemoteWipeAsync()` requires a connection to IoT Hub and it will update the device twin with its status.

**Example**

<pre>
    async Task OnRemoteWipeClicked(DeviceManagementClient dmClient)
    {
        await dmClient.StartRemoteWipeAsync(true);
    }
</pre>

### DeviceManagement.StartRemoteWipeAsync()

<pre>
    <b>Namespace</b>:
	Microsoft.Devices.Management
</pre>

<pre>
    <b>Class</b>:
    DeviceManagement
</pre>

<pre>
    <b>Methods</b>:
    public async Task StartRemoteWipeAsync(bool clearTPM)
</pre>

Notes:

- `DeviceManagement.StartRemoteWipeAsync()` does not require a connection to IoT Hub and it will not update the device twin with its status.

**Example**

<pre>
    async Task OnRemoteWipeClicked(DeviceManagementClient dmClient)
    {
        DeviceManagement dm = DeviceManagement.CreateWithoutAzure();
        await dm.StartRemoteWipeAsync(true);
    }
</pre>

### windows.startRemoteWipeAsync

This interface is asynchronous and will return before completing the operation. The status can be tracked through the device twin `remoteWipe` node (see below).

#### Input Payload 

<pre>
{
    "clearTPM" : "<i>see below</i>"
}
</pre>

- ```"clearTPM"``` : When set to ```"true"```, all TPM slots will be cleared - otherwise, TPM will be not be cleared.

#### Output Payload

`windows.startRemoteWipeAsync` returns a <i>Status Object</i> (see [Status Reporting](status-reporting.md)).

<pre>
{
    "status" : {
        &lt;<i>Status Object</i>&gt;
    }
}
</pre>

For details on <i>Status Object</i>, see [Error Reporting](error-reporting.md).

#### Device Twin Reporting

As the method executes, it will also update the device twin with its current status and its final state.

<pre>
"reported" : {
    "windows" : {
        "remoteWipe": {
            "lastChange" = &lt;<i>Status Object</i>&gt;
        }
    }
}
</pre>

#### Example

**Input**

<pre>
{
    "clearTPM" : "true"
}
</pre>

**Output**
<pre>
{
    "status" : {
        "time" : "<i>time stamp</i>",
        "state": "pending",
    }
}
</pre>

**Device Twin Reporting**

<pre>
"reported" : {
    "windows" : {
        "remoteWipe": {
            "lastChange": {
                "time" : "<i>time stamp</i>",
                "state": "pending",
            },
            "response": "scheduled"
    }
}
</pre>

And then, upon success, it will be changed to:

<pre>
"reported" : {
    "windows" : {
        "remoteWipe": {
            "lastChange": {
                "time" : "<i>time stamp</i>",
                "state": "completed",
            },
            "response": "scheduled"
        }
    }
}
</pre>

----

[Home Page](../README.md) | [Library Reference](library-reference.md)