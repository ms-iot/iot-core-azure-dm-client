## Immediate Reboot

The **Immediate Reboot** operation is initiated by the device receiving the `microsoft.management.immediateReboot` method.

## Input Payload 
Input payload is empty

## Output Payload
The device responds immediately with the following JSON payload:

| Key           | Value |
| ------------- |-------|
| `"response"`  | _See below_ |

Possible `"response"` values of are: 
- `"accepted"` - The reboot request was accepted. The device will attempt to reboot momentarily (note: the attempt might fail, see below)
- `"rejected"` - The device rejected the reboot request. The device will not reboot.
- `"scheduled"`- The reboot request was accepted. The device will reboot at some time in the future.

The device might not be in a state that allows reboot. For example, it might be
in a middle of an important operation than cannot be disrupted. If no such
condition is detected, the device responds by accepting the request and
attempting to reboot.

Further, the device management client needs to consult the primary app running
on the device whether it is acceptable to reboot. The app responds based on its
business logic which, for interactive apps, might require user consent.

The state of the latest reboot attempt is communicated to the back-end via
reported properties as described in [Device Twin Communication](#device-twin-communication) below.

**Examples:**

Successful response:

```
"response" : "accepted"
```

## Device Twin Communication

After responding to the method call, the device attempts to reboot. The result
of the attempt is recorded in the reported property `"lastRebootAttempt"`, which
is JSON object with two key/value pairs defined as follows:

| Key        | Value |
| ---------- |-------|
| `"time"`   | Datetime in ISO 8601 format, UTC |
| `"status"` | `"success"` or `"failure"` |

*After* the device boots, the `"lastBootTime"` property is set, which is defined as follows:

| Key        | Value |
| ---------- |-------|
| `"lastBootTime"`   | Datetime in ISO 8601 format, UTC |


Note that this property is set after every boot, whether it was initiated by
the Immediate Reboot, [Scheduled Reboot](scheduled-reboot.md) or any other reason.

**Examples:**

Successful response:

```
"lastRebootAttempt" : {
    "time" : "2017-01-25T13:27:33+04:00",
    "status" : "success"
}

```

Device boots and sets `"lastBootTime"` property:

```
`"lastBootTime"` : "2017-01-25T13:27:33+04:00"

```
