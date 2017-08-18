# Device Factory Reset

A **Device Factory Reset** re-applies a pre-stored Windows image (from the recovery partition) to the OS partition. It also allows the clearing of the TPM.

The **Device Factory Reset** operation is initiated by either:

- The application calling `StartFactoryResetAsync()`. 
- The operator invoking the Azure direct method `windows.startFactoryResetAsync`.

### StartFactoryResetAsync()

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

**Example**

<pre>
    async Task OnRebootClicked(DeviceManagementClient dmClient)
    {
        await dmClient.StartFactoryResetAsync(true, "recoveryPartitionGUID");
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

This is the json payload returned by the method.

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
            "time" : "<i>time stamp</i>",
            "state": "pending",
        }
    }
}
</pre>

And then, upon success, it will be changed to:

<pre>
"reported" : {
    "windows" : {
        "factoryReset": {
            "time" : "<i>time stamp</i>",
            "state": "committed",
        }
    }
}
</pre>


----

[Home Page](../README.md) | [Library Reference](library-reference.md)