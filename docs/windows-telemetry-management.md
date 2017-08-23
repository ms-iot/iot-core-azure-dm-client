# Windows Telemetry Management

The **Windows Telemetry Management** allows control over what telemetry data Windows may send to Microsoft.

## Device Twin Interface

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

## .Net Interface

<pre>
    <b>Namespace</b>:
    Microsoft.Devices.Management
</pre>

<pre>
    <b>Enums</b>:
    public enum WindowsTelemetryLevel
    {
        Security,
        Basic,
        Enhanced,
        Full
    }
</pre>

For details about what each of the supported values mean, see [MSDN](https://docs.microsoft.com/en-us/windows/configuration/configure-windows-telemetry-in-your-organization).

<pre>
    <b>Class</b>:
    DeviceManagementClient
</pre>

<pre>
    <b>Methods</b>:
        public async Task SetWindowsTelemetryLevelAsync(WindowsTelemetryLevel level);
        public async Task&lt;WindowsTelemetryLevel&gt; GetWindowsTelemetryLevelAsync();
</pre>

----

[Home Page](../README.md) | [Library Reference](library-reference.md)