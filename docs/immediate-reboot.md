## Reboot

The **reboot** operation is initiated by the device receiving `microsoft.management.reboot` method.

## Input Payload 
Input payload is specified as follows:

| Parameters    | Required | Description                                                                         |
| ------------- |----------|:------------------------------------------------------------------------------------|
| `"type"`      | Yes      | Possible values are either `"immediate"` or one of the JSON objects specified below |

Non-immediate reboot type parameter must be a JSON object represented as a single key/value pair
where the keys is either `"one-time"` or `"periodic"`:

| Parameters    | Required | Description             |
| ------------- |----------|:------------------------|
| `"one-time"`  | Yes      | Datetime in ISO 8601 format, UTC |

| Parameters    | Required | Description             |
| ------------- |----------|:------------------------|
| `"periodic"`  | Yes      | JSON object described below |

Periodic reboot is specified as a single key/value pair JSON object where the key is either `"daily"` or `"weekly"`, as described below:

| Parameters    | Required | Description             |
| ------------- |----------|:------------------------|
| `"daily"`     | No       | Local time in *hh::mm* format |
| `"weekly"`    | No       | JSON object described below   |

Weekly reboot is specified as a JSON object with keys specified as follows:

| Parameters    | Required  | Description             |
| ------------- |-----------|:------------------------|
| `"day"`       | Yes       | One of `"Mon, `"Tue"`, `"Wed"`, `"Thur"`, `"Fri"`, `"Sat"` or `"Sun"` |
| `"time"`      | Yes       | Local time in *hh::mm* format  |

### Examples

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
        "weekly" {
            "day" : "Sun",
            "time" : "02:00"
        }
    }
}
```

## Output Payload

TBD

## Device Twin Communication

TBD
