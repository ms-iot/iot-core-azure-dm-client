# Device Factory Reset

A **Device Factory Reset** re-applies a pre-stored Windows image (from the recovery partition) to the OS partition. It also allows the clearing of the TPM.

The **Device Factory Reset** operation is initiated by either:

- the application calling `FactoryResetAsync()`. 
- the operator invoking the Azure direct method `windows.factoryReset`.

### FactoryResetAsync()

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
    public async Task FactoryResetAsync(bool clearTPM, string recoveryPartitionGUID)
</pre>

**Example**

<pre>
    async Task OnRebootClicked(DeviceManagementClient dmClient)
    {
        await dmClient.FactoryResetAsync(true, "recoveryPartitionGUID");
    }
</pre>

### windows.factoryReset

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
<pre>
{
    "response" : "<i>see below</i>"
    "reason" : "<i>see below</i>"
}
</pre>

- ```"response"``` : This can either be set to ```"accepted"``` or ```"rejected"```. In case it is set to ```"rejected"```, the ```"reason"``` field will contain more details on why the method has been rejected..
- ```"reason"``` : If the ```"response"``` is ```"accepted"```, this will be empty - otherwise, this field will contain more details on why the method has been rejected.

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
    "response" : "accepted"
    "reason" : ""
}
</pre>

----

[Home Page](../README.md) | [Library Reference](library-reference.md)