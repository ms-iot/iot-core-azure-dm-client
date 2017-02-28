# Device management interfaces

Windows IoT Core devices can be managed via Azure IoT Hub. All device management operation are implemented via the Azure IoT Hub [direct methods](<https://docs.microsoft.com/en-us/azure/iot-hub/iot-hub-devguide-direct-methods>) and the [device twin](<https://docs.microsoft.com/en-us/azure/iot-hub/iot-hub-devguide-device-twins>).


## Direct Method Format

Some device management actions are initiated by direct methods. Such methods start with the `microsoft.management` prefix followed by the method name. The method payload (when non-empty) is in JSON format. The return payload (if not empty) is also in JSON format.

Example:

```
microsoft.management.transmogrify
```

The payload for a method can look as follows:
```
"parameter" : "value"
```

The exact specification for each device management operation is defined in the [Specification](#specification) below.

## Device Twin Format

Certain device management operations are initiated by desired properties set from the IoT Hub. For example, some configuration settings are set by the desired properties as depicted in the example below:

```
"desired": {
    "microsoft" : { 
        "management" : {
            "key1" : value1,
            "key2" : value2,
            ...
        }
    }
    ...
}
```

## Specification

The specification for each operation is provided below.

- [Report All Device Properties](report-all-device-properties.md)
- [Immediate Reboot](immediate-reboot.md)
- [Scheduled Reboot](scheduled-reboot.md)
- [Install App](install-app.md)
- [Start DM Application Self-Update](app-self-update.md)
- [Manage Certificates](certificate-management.md)
