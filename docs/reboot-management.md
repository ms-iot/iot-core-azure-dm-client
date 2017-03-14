#Reboot Management

The **Reboot Management** functionality allows the operator to perform the following tasks:

- Retrieve last reboot time.
- Allow/Disallow system reboots.
- Initiate immediate reboots
  - Issue command and inspect its status in the device twin.
- Schedule reboots
  - Schedule a reboot for a certain time - to be executed once, or daily - and inspect that schedule in the device twin.

## Retrieve Last Boot Time

*After* the device reboots, the `"reported.microsoft.management.rebootInfo.lastBootTime"` property is set, which is defined as follows:

<pre>
"reported" : {
    "microsoft" : {
        "management" : {
            "rebootInfo" : {
                "lastBootTime" : "<i>Datetime in ISO 8601 format, UTC</i>"
            }
        }
    }
}
</pre>

**Example**

Device boots and sets `"lastBootTime"` property:

```
"reported" : {
    "microsoft" : {
        "management" : {
            "rebootInfo" : {
                `"lastBootTime"` : "2017-01-25T13:34:33+04:00"
            }
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
    public enum RebootRequestStatus
    {
        Allowed,
        Disabled,
        InActiveHours,
        RejectedByApp
    }
</pre>

<pre>
    <b>Class</b>:
	DeviceManagementClient
</pre>

<pre>
    <b>Methods</b>:
    public async Task AllowReboots(bool allowReboots)
    public async Task&lt;RebootRequestStatus&gt; IsRebootAllowedBySystem()
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

## Initiate Immediate Reboot

The **Immediate Reboot** operation is initiated by the device receiving the `microsoft.management.immediateReboot` method.

### Input Payload 
Input payload is empty

### Output Payload
The device responds immediately with the following JSON payload:

<pre>
"response" : "<i>see below</i>"
</pre>

Possible `"response"` values are: 

- `"accepted"` - The reboot request was accepted. The device will attempt to reboot momentarily (note: the attempt might fail, see below)
- `"disabled"` - is returned when the application flags its busy state by calling `"AllowReboots(false)"`.
- `"inActiveHours"` - is returned when the immediate reboot command is received between the active hours as 
   specified by `windowsUpdatePolicy` (see [Windows Update Management](windows-update-management.md) 
   `desired.microsoft.management.windowsUpdatePolicy.activeHoursStart` and `desired.microsoft.management.windowsUpdatePolicy.activeHoursEnd`).

The state of the latest reboot request is communicated to the back-end via
reported properties as described in [Device Twin Communication](#device-twin-communication) below.

Note that an immediate roboot request will be 'accepted' initially if it meets the current policy set on the device 
(namely; outside active hours, and reboot are not disallowed by the application) - however, the request might still
be rejected later when the application is intorregated (where it may prompt the application user for a response; for example).
Such rejection will be expressed in the Device Twin.

After the device reboots, `"reported.microsoft.management.rebootInfo.lastBootTime"` will be set to a new value.
This can be used to confirm the reboot took place.

**Examples:**

Successful response:

```
"response" : "accepted"
```

### Device Twin Communication

After responding to the method call, the device attempts to reboot. The result
of the attempt is recorded in the reported property `"reported.microsoft.management.rebootInfo.lastRebootCmdTime"`, which
is JSON object with two key/value pairs defined as follows:

<pre>
"reported" : {
    "microsoft" : {
        "management" : {
            "rebootInfo" : {
                "lastRebootCmdTime": "<i>Datetime in ISO 8601 format, UTC</i>"
                "lastRebootCmdStatus": "<i>value</i>"
            }
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

```
"reported" : {
    "microsoft" : {
        "management" : {
            "rebootInfo" : {
                "lastRebootCmdTime": "2017-01-25T13:27:33+04:00"
                "lastRebootCmdStatus": "accepted"
            }
        }
    }
}
```

## Schedule Reboots

The **Schedule Reboots** operation is initiated by the device receiving the `"desired.microsoft.management.scheduledReboot"` desired property.

### Configuration Format
The format of the `"desired.microsoft.management.scheduledReboot"` desired property is as follows:

<pre>
"desired" : {
    "microsoft" : {
        "management" : {
            "scheduledReboot" :{
                "singleRebootTime" : "<i>Datetime in ISO 8601 format, UTC</i>"
                "dailyRebootTime" : "<i>Datetime in ISO 8601 format, UTC</i>"
            }
        }
    }
}
</pre>

Note that the full date and time are required when specifying the daily reboot time. The date will denote when the daily reboot should start taking place.

**Examples**

Perform a singleRebootTime reboot on Jan 25th, 2017 at 09:00 UTC time and also reboot daily at 3 AM:

```
"desired" : {
    "microsoft" : {
        "management" : {
            "scheduledReboot" : {
                "singleRebootTime" : "2017-01-25T09:00:00+00:00"
                "dailyRebootTime" : "2017-01-25T03:00:00+00:00"
            }
        }
    }
}
```

Either or both <i>singleRebootTime</i> and <i>dailyRebootTime</i> can be set to an empty string to indicate no reboot is scheduled.

### Reporting Format

The current state of reboot configuration and status is stored under the `"reported.microsoft.management.rebootInfo"`.

<pre>
"reported" : {
    "microsoft" : {
        "management" : {
            "rebootInfo" : {
                "dailyRebootTime": "<i>Datetime in ISO 8601 format, UTC</i>",
                "singleRebootTime": "<i>Datetime in ISO 8601 format, UTC</i>"
            }
        }
    }
}
</pre>

**dailyRebootTime** and **singleRebootTime** reflect the values set in the desired scheduledReboot property.