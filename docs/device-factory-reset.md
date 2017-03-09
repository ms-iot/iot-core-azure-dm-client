# Device Factory Reset

A **Device Factory Reset** re-applies a pre-stored Windows image (from the recovery partition). It also allows the clearing of the TPM.

### Method

<pre>microsoft.management.factoryReset
</pre>

### Input

<pre>
{
    "recoveryPartitionGUID" : "<i>recoveryPartitionGuid</i>"
    "clearTPM" : "<i>see below</i>"
}
</pre>

- ```"recoveryPartitionGUID"``` : This is the guid of the partition that contains the Windows image file (wim) to be re-applied.
- ```"clearTPM"``` : When set to ```"true"```, all TPM slots will be cleared - otherwise, TPM will be not be cleared.

### Output
<pre>
{
    "response" : "<i>see below</i>"
    "reason" : "<i>see below</i>"
}
</pre>

- ```"response"``` : This can either be set to ```"accepted"``` or ```"rejected"```. In case it is set to ```"rejected"```, the ```"reason"``` field will contain more details on why the method has been rejected..
- ```"reason"``` : If the ```"response"``` is ```"accepted"```, this will be empty - otherwise, it will field will contain more details on why the method has been rejected.

## Example

### Input

<pre>
{
    "recoveryPartitionGUID" : "<i>"66F49469-6F65-4E1D-9891-2EB2E83C8B85"</i>"
    "clearTPM" : "true"
}
</pre>

### Output
<pre>
{
    "response" : "accepted"
    "reason" : ""
}
</pre>
