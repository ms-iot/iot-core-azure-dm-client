# Windows Telemetry Management

The **Windows Telemetry Management** allows control over what telemetry data Windows may send to Microsoft.

### Configuration

The **Windows Update Policy** can be configured through the ```"windowsUpdatePolicy"``` node in the desired properties section as follows:

<pre>
"desired" : {
    "windows" : {
        "windowsTelemetry": {
            "level": "security|basic|enhanced|full"
            }
        }
    }
}
</pre>

For details about what each of the supported values mean, see [MSDN](https://docs.microsoft.com/en-us/windows/configuration/configure-windows-telemetry-in-your-organization).

### Reporting

<pre>
"reported" : {
    "windows" : {
        "windowsTelemetry": {
            "level": "security|basic|enhanced|full"
            }
        }
    }
}
</pre>

For details about what each of the supported values mean, see [MSDN](https://docs.microsoft.com/en-us/windows/configuration/configure-windows-telemetry-in-your-organization).

----

[Home Page](../README.md) | [Library Reference](library-reference.md)