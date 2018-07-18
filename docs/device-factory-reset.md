# Device Factory Reset

A **Device Factory Reset** re-applies a pre-stored Windows image (from the recovery partition) to the OS partition. It also allows the clearing of the TPM.

The **Device Factory Reset** operation is initiated by either:

- The application calling `DeviceManagementClient.StartFactoryResetAsync()`. 
- The application calling `DeviceManagement.StartFactoryResetAsync()`. 
- The operator invoking the Azure direct method `windows.startFactoryResetAsync`.

### DeviceManagementClient.StartFactoryResetAsync()

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
    public async Task StartFactoryResetAsync(bool clearTPM, string recoveryPartitionGUID)
</pre>

Notes:

- `DeviceManagementClient.StartFactoryResetAsync()` requires a connection to IoT Hub and it will update the device twin with its status.

**Example**

<pre>
    async Task OnRebootClicked(DeviceManagementClient dmClient)
    {
        await dmClient.StartFactoryResetAsync(true, "&lt;<i>recoveryPartitionGUID</i>&gt;");
    }
</pre>

### DeviceManagement.StartFactoryResetAsync()

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
    public async Task StartFactoryResetAsync(bool clearTPM, string recoveryPartitionGUID)
</pre>

Notes:

- `DeviceManagement.StartFactoryResetAsync()` does not require a connection to IoT Hub and it will not update the device twin with its status.

**Example**

<pre>
    async Task OnRebootClicked(DeviceManagementClient dmClient)
    {
        DeviceManagement dm = DeviceManagement.CreateWithoutAzure();
        await dm.StartFactoryResetAsync(true, "&lt;<i>recoveryPartitionGUID</i>&gt;");
    }
</pre>

### windows.startFactoryResetAsync

This interface is asynchronous and will return before completing the operation. The status can be tracked through the device twin `factoryReset` node (see below).

#### Input Payload 

<pre>
{
    "recoveryPartitionGUID" : "<i>recoveryPartitionGuid</i>"
    "clearTPM" : "<i>see below</i>"
}
</pre>

- ```"recoveryPartitionGUID"``` : This is the guid of the partition that contains the Windows image file (wim) to be re-applied.
- ```"clearTPM"``` : When set to ```"true"```, all TPM slots will be cleared - otherwise, TPM will be not be cleared.

#### Output Payload

`windows.startFactoryResetAsync` returns a <i>Status Object</i> (see [Status Reporting](status-reporting.md)).

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
        "factoryReset": {
            "lastChange" = &lt;<i>Status Object</i>&gt;
        }
    }
}
</pre>

#### Example

**Input**

<pre>
{
    "recoveryPartitionGUID" : "<i>"66F49469-6F65-4E1D-9891-2EB2E83C8B85"</i>"
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
        "factoryReset": {
            "lastChange": {
                "time" : "<i>time stamp</i>",
                "state": "pending"
            }
        }
    }
}
</pre>

And then, upon success, it will be changed to:

<pre>
"reported" : {
    "windows" : {
        "factoryReset": {
            "lastChange": {
                "time" : "<i>time stamp</i>",
                "state": "completed"
            }
        }
    }
}
</pre>

----

[Home Page](../README.md) | [Library Reference](library-reference.md)