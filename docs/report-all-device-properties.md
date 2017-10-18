# Report All Device Properties

When a desired device management property changes in the device twin, the device management client gets notified and does two things:

- Applies the new desired state.
- Reads the current device state (that corresponds to the desired state) and reports it to the device twin.

For example, if the operator sets the time zone to be <i>Mountain Time</i>, the device management client will:

- Set the time zone to Mountain Time.
- Read the time zone from the device, and report it to the device twin.

The operator (or more specifically, the code running on in the Portal) can then compare the desired state to the reported state and decide whether the device is compliant or not.

This is a conservative reporting approach and is meant to report only the properties that are configured (rather than reporting everything the device can report).

Should the operator want to force the device to report all its properties, `windows.reportAllAsync` method can be invoked.

## windows.reportAllAsync

Reports all properties supported by the Windows IoT Azure Device Client.

#### Input Payload 
Input payload is empty

#### Output Payload

This method is asynchronous, so it returns immediately and sets the <i>Status Object</i> to `pending`. For more details on the <i>Status Object</i>, see [Status Reporting](status-reporting.md).

<pre>
{
    "status" : {
        &lt;<i>Status Object</i>&gt;
    }
}
</pre>

----

[Home Page](../README.md) | [Library Reference](library-reference.md)