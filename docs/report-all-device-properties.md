## Report All Device Properties

The **Report All Device Properties** operation is initiated by the device receiving the `microsoft.management.reportAllDeviceProperties` method.

## Input Payload 
Input payload is empty

## Output Payload
The device responds immediately with the following JSON payload:

<pre>
"response" : value (<i>See below</i>)
</pre>

Possible `"response"` values are: 
- `"success"` - The request was accepted and scheduled. The device will attempt to report all properties momentarily (note: the attempt might fail, see below).
- `"failure"` - The device failed to schedule reporting all the properties.

**Examples:**

Successful response:

```
"response" : "success"
```
