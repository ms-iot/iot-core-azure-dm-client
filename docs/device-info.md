# Device Information

The **Device Information** functionality allows the operator to inspect various device properties.

The format of the `"reported.windows.deviceInfo"` reported property is as follows:

<pre>
"reported" : {
    "windows": {
        "deviceInfo": {
            "osEdition": "<i>OS edition</i>",
            "secureBootState": "<i>secureBootStateValue</i>",
            "totalMemory": "<i>available memory in MBs</i>",
            "totalStorage": "<i>available storage in MBs</i>",
            "name": "<i>machine name</i>",
            "processorArchitecture": "<i>processArchitecture</i>",
            "commercializationOperator": "<i>commercializationOperator</i>",
            "displayResolution": "<i>WidthXHeight</i>",
            "radioSwVer": "<i>radio software version</i>",
            "processorType": "<i>processorType</i>",
            "platform": "IoTUAP",
            "osVer": "<i>OS version</i>",
            "fwVer": "<i>firmware version</i>",
            "hwVer": "<i>hardware version</i>",
            "oem": "<i>oem</i>",
            "type": "<i>device type</i>",
            "lang": "<i>lanugage</i>",
            "dmVer": "<i>device management version</i>",
            "model": "<i>device model</i>",
            "manufacturer": "<i>manufacturer</i>",
            "id": "<i>device id guid</i>"
            "batteryStatus": "<i>batteryStatus</i>",
            "batteryRemaining": "<i>batteryRemaining</i>",
            "batteryRuntime": "<i>batteryRuntime</i>",
        }
    }
}
</pre>

**Notes**

`"secureBootStateValue"` can be:
- 0 - Not supported
- 1 - Enabled
- 2 - Disabled

`"commercializationOperator"`:
- the name of the mobile operator if it exists; otherwise it returns 404

`"processorType"`:
- ToDo: missing documentation/incorrect CSP implementation (https://msdn.microsoft.com/en-us/windows/hardware/commercialize/customize/mdm/devdetail-csp).

`"batteryStatus"`:
- ToDo: undocumented (https://msdn.microsoft.com/en-us/windows/hardware/commercialize/customize/mdm/devdetail-csp).

`"batteryRemaining"` :
- The value is the number of seconds of battery life remaining when the device is not connected to an AC power source. When it is connected to a power source, the value is -1. When the estimation is unknown, the value is -1.

`"batteryRuntime"` :
- ToDo: undocumented (https://msdn.microsoft.com/en-us/windows/hardware/commercialize/customize/mdm/devdetail-csp).

**Examples**

To configure the device to Pacifict Standard Time, the <i>timeInfo</i> is set to:

<pre>
"reported" : {
    "windows": {
        "deviceInfo": {
            "osEdition": "123",
            "secureBootState": "0",
            "totalMemory": "1024",
            "totalStorage": "1448",
            "name": "gmileka0222",
            "processorArchitecture": "5",
            "commercializationOperator": "000-88",
            "displayResolution": "1600x1200",
            "radioSwVer": "Not Present",
            "processorType": "0",
            "platform": "IoTUAP",
            "osVer": "10.0.15143.1000",
            "fwVer": "10.0.14961.0",
            "hwVer": "0.1",
            "oem": "Raspberry Pi",
            "type": "Raspberry Pi 2",
            "lang": "en-US",
            "dmVer": "1.3",
            "model": "Raspberry Pi 2 Model B",
            "manufacturer": "RASPBERRY PI",
            "id": "{E9F452D0-0288-4AAC-BC5B-A380500A3DC9}"
            "batteryStatus": "1",
            "batteryRuntime": "-1",
            "batteryRemaining": "-1",
        }
    }
}
</pre>

----

[Home Page](../README.md) | [Library Reference](library-reference.md)