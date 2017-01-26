## Scheduled Reboot

The **Scheduled Reboot** operation is initiated by the device receiving the `"desired.microsoft.management.scheduledReboot"` desired property.

## Format
The format of the `"desired.microsoft.management.scheduledReboot"` desired property is as follows:

<pre>
"desired" : {
    "microsoft" : {
        "management" : {
            "scheduledReboot" : <i>See below</i>
        }
    }
}
</pre>

The possible values of the `"scheduledReboot"` key are as follows:

 - For one-time reboot:
 
 <pre>
 {
     "one-time" : "<i>Datetime in ISO 8601 format, UTC</i>"
 }
 </pre>

- For daily reboot:

 <pre>
 {
     "daily" : "<i>Local time in <b>hh::mm</b> format</i>"
 }
 </pre>

**Examples**

Perform a one-time reboot on Jan 25th, 2017 at 09:00 UTC time

```
"desired" : {
    "microsoft" : {
        "management" : {
            "scheduledReboot" : {
                "one-time" : "2017-01-25T09:00:00+00:00"
            }
        }
    }
}
```
Start performing daily reboots at 3 AM:

```
"desired" : {
    "microsoft" : {
        "management" : {
            "scheduledReboot" : {
                "daily" : "03:00"
            }
        }
    }
}
```

## Device Twin Communication

The communication via the device twin is the same as for the 
[Immediate Reboot](immediate-reboot.md).
