## Scheduled Reboot

The **Scheduled Reboot** operation is initiated by the device receiving the `"scheduledReboot"` desired property.

## Format
The format of the `"scheduledReboot"` desired property is as follows:

| Parameters    | Required | Description                         |
| ------------- |----------|:------------------------------------|
| `"type"`      | Yes      | Possible values are specified below |

The `"type"` parameter must be a JSON object represented as a single key/value pair
where the keys is either `"one-time"` or `"daily"`. The former is specified as follows:

| Parameters    | Required | Description             |
| ------------- |----------|:------------------------|
| `"one-time"`  | No       | Datetime in ISO 8601 format, UTC |

Daily reboot specifies time to reboot:

| Parameters    | Required | Description             |
| ------------- |----------|:------------------------|
| `"daily"`     | No       | Local time in *hh::mm* format |

**Examples**

Perform an immediate reboot:

```
"type" : "immediate"
```

Perform a one-time reboot on Jan 25th, 2017 at 09:00 UTC time

```
"type" : {
   "one-time" : "2017-01-25T09:00:00+00:00"
}
```
Start performing daily reboots at 3 AM:

```
"type" : {
    "periodic" : {
        "daily" : "03:00"
    }
}
```

Start performing weekly reboots each Sunday at 2 AM:

```
"type" : {
    "periodic" : {
        "weekly" : {
            "day" : "Sun",
            "time" : "02:00"
        }
    }
}
```

## Device Twin Communication

The communication via the device twin is the same as for the 
[Immediate Reboot](immediate-reboot.md).
