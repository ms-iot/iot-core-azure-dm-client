# Reboot Management

The **Reboot Management** functionality can perform the following tasks:

- Retrieve last reboot time.
- Allow/Disallow system reboots (also available as a .NET API).
- Initiate immediate reboots (also available as a .NET API).
  - Issue command and inspect its status in the device twin.
- Schedule reboots
  - Schedule a reboot for a certain time - to be executed once, or daily - and inspect that schedule in the device twin.

## Retrieve Last Boot Time

*After* the device reboots, the `"reported.windows.rebootInfo.lastBootTime"` property is set, which is defined as follows:

<pre>
"reported" : {
    "windows" : {
        "rebootInfo" : {
            "lastBootTime" : "<i>Datetime in ISO 8601 format, UTC</i>"
        }
    }
}
</pre>

**Example**

Device boots and sets `"lastBootTime"` property:

```
"reported" : {
    "windows" : {
        "rebootInfo" : {
            `"lastBootTime"` : "2017-01-25T13:34:33+04:00"
        }
    }
}
```

Note that this property is set after every boot, regardless of how the reboot was initiated.

## Allow/Disallow System Reboots

The device might not be in a state that allows reboot. For example, it might be in the middle of an important operation that cannot be disrupted.
In such cases, the application can disallow system reboots until the important operation is done.

This policy applies to:

- Device Management immediate reboot commands.
- Windows update reboots.

This policy does not apply to:

- Scheduled reboots. (ToDo)

To allow/disallow reboots, the application developer can invoke the following .Net APIs.

<pre>
    <b>Namespace</b>:
    Microsoft.Devices.Management
</pre>

<pre>
    <b>Enums</b>:
    public class RebootCmdDataContract
    {
        public enum ResponseValue
        {
            Allowed,
            Scheduled,
            Disabled,
            InActiveHours,
            RejectedByApp
        }
    }
</pre>

<pre>
    <b>Class</b>:
    DeviceManagementClient
</pre>

<pre>
    <b>Methods</b>:
    public async Task AllowReboots(bool allowReboots)
    public async Task&lt;RebootCmdDataContract.ResponseValue&gt; IsRebootAllowedBySystem()
</pre>

Note that IsRebootAllowedBySystem() can return only one of the following values `"Allowed"`, `"Disabled"`, or `"InActiveHours"`.

**Example**

<pre>
    async Task OnCriticalTaskStart(DeviceManagementClient dmClient)
    {
        await dmClient.AllowReboots(false);
        return CriticalTaskStart();
    }

    void OnCriticalTaskFinished(DeviceManagementClient dmClient)
    {
        dmClient.AllowReboots(true);
    }
</pre>

## Initiate Reboot

The **Reboot Command** operation is initiated by either:

- the application calling `RebootAsync()`. 
- the operator invoking the Azure direct method `windows.rebootAsync`.

### RebootAsync()

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
    public async Task RebootAsync()
</pre>

**Example**

<pre>
    async Task OnRebootClicked(DeviceManagementClient dmClient)
    {
        await dmClient.RebootAsync();
    }
</pre>

### windows.rebootAsync

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

#### Device Twin Reporting

The outcome of executing the reboot command will be reported to the device twin under `rebootCmd`.

<pre>
    "rebootCmd": {
        "lastChange": {
            "time": "2017-08-21T02:58:56.815724Z",
            "state": "completed"
        },
        "response": "scheduled"
    },
</pre>

Possible `"response"` values are: 

- `"scheduled"` - The reboot request has been scehduled in 5 minutes. The device will attempt to reboot momentarily (note: the attempt might fail, see below)
- `"disabled"` - is returned when the application flags its busy state by calling `"AllowReboots(false)"`.
- `"inActiveHours"` - is returned when the immediate reboot command is received between the active hours as 
   specified by `windowsUpdatePolicy` (see [Windows Update Management](windows-update-management.md) 
   `desired.windows.windowsUpdatePolicy.activeHoursStart` and `desired.windows.windowsUpdatePolicy.activeHoursEnd`).

Note that an immediate roboot request will be 'accepted' initially if it meets the current policy set on the device 
(namely; outside active hours, and reboot are not disallowed by the application) - however, the request might still
be rejected later when the application is intorregated (where it may prompt the application user for a response; for example).

After the device reboots, `"reported.windows.rebootInfo.lastBootTime"` will be set to a new value.
That value can be used to confirm the reboot took place.

#### Device Twin Communication

After responding to the method call, the device attempts to reboot. The result
of the attempt is recorded in the reported property `"reported.windows.rebootInfo.lastRebootCmdTime"`, which
is JSON object with two key/value pairs defined as follows:

<pre>
"reported" : {
    "windows" : {
        "rebootInfo" : {
            "lastRebootCmdTime": "<i>Datetime in ISO 8601 format, UTC</i>"
            "lastRebootCmdStatus": "<i>value</i>"
        }
    }
}
</pre>

`"lastRebootCmdTime"` is persisted on the device. It reflects the last time the device received an immediate reboot command. 

`"lastRebootCmdStatus"` possible values are:

- `"accepted"` - The reboot request was accepted. The device will attempt to reboot momentarily.
- `"disabled"` - <i>See above</i>.
- `"inActiveHours"` - <i>See above</i>.
- `"rejectedByApp"` - is returned when the application is interrogated at the time the request is received, 
   and rejects the reboot command. The application can be interrogated by implementing `IDeviceManagementRequestHandler.IsSystemRebootAllowed`.
   The applications's response here is based on its own business logic and might require user consent.

**Examples:**

Successful response:

<pre>
"reported" : {
    "windows" : {
        "rebootInfo" : {
            "lastRebootCmdTime": "2017-01-25T13:27:33+04:00"
            "lastRebootCmdStatus": "accepted"
        }
    }
}
</pre>

## Scheduled Reboots

The **Schedule Reboot** can be configured using the `rebootInfo` node in the desired properties section.

### Configuration Format
The format of the `"desired.windows.rebootInfo"` desired property is as follows:

<pre>
"desired" : {
    "windows" : {
        "rebootInfo" :{
            "singleRebootTime" : "<i>Datetime in ISO 8601 format, UTC</i>"
            "dailyRebootTime" : "<i>Datetime in ISO 8601 format, UTC</i>"
        }
    }
}
</pre>

Note that the full date and time are required when specifying the daily reboot time. The date will denote when the daily reboot should start taking place.

**Examples**

Perform a singleRebootTime reboot on Jan 25th, 2017 at 09:00 UTC time and also reboot daily at 3 AM:

<pre>
"desired" : {
    "windows" : {
        "rebootInfo" : {
            "singleRebootTime" : "2017-01-25T09:00:00+00:00"
            "dailyRebootTime" : "2017-01-25T03:00:00+00:00"
        }
    }
}
</pre>

Either or both <i>singleRebootTime</i> and <i>dailyRebootTime</i> can be set to an empty string to indicate no reboot is scheduled.

### Reporting Format

The current state of reboot configuration and status is stored under the `"reported.windows.rebootInfo"`.

<pre>
"reported" : {
    "windows" : {
        "rebootInfo" : {
            "dailyRebootTime": "<i>Datetime in ISO 8601 format, UTC</i>",
            "singleRebootTime": "<i>Datetime in ISO 8601 format, UTC</i>"
        }
    }
}
</pre>

----

[Home Page](../README.md) | [Library Reference](library-reference.md)