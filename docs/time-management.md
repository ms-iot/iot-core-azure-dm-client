# Time Management

The **Time** functionality allows the operator to configure the following:

- The time zone.
- The time synchronization server.
- The Time Service on the Windows machine.

### Time Zone and Synchronization Server

#### Setting
The format of the `"desired.windows.timeInfo"` desired property is as follows:

<pre>
"desired" : {
    "windows": {
        "timeInfo": {
            "ntpServer": "<i>ntpServer</i>",
            "dynamicDaylightTimeDisabled": false|true,
            "timeZoneKeyName": "<i>registry key name</i>",
            "timeZoneBias": <i>bias</i>,
            "timeZoneDaylightBias": <i>daylightBias</i>,
            "timeZoneDaylightDate": "<i>Datetime in ISO 8601 format, UTC</i>",
            "timeZoneDaylightName": "<i>daylight display name</i>",
            "timeZoneDaylightDayOfWeek": <i>0 for Sunday, etc</i>,
            "timeZoneStandardBias": <i>standardBias</i>,
            "timeZoneStandardDate": "<i>Datetime in ISO 8601 format, UTC</i>",
            "timeZoneStandardName": "<i>standard display name</i>",
            "timeZoneStandardDayOfWeek": <i>0 for Sunday, etc</i>
            }
        }
    }
}
</pre>

**Note:**

- This interface (except for the ntp server part) exposes [SetDynamicTimeZoneInformation](https://msdn.microsoft.com/en-us/library/windows/desktop/ms724932.aspx).
- When `dynamicDaylightTimeDisabled` is set to 
  - `false` (i.e. dynamic daylight is enabled), `timeZoneKeyName` is used to locate the settings in the registery on the device. If it cannot be found under that `timeZoneKeyName` registry key, it fallbacks to use the detailed time zone fields provided above.
  - `true` (i.e. dynamic daylight is disabled), the detailed time zone fields are  used to set the current timezone.

#### Reporting
The format of the `"reported.microsoft.management.timeInfo"` desired property is as follows:

<pre>
"reported" : {
    "windows": {
        "timeInfo": {
            "ntpServer": "<i>ntpServer</i>",
            "localTime": "<i>Datetime in ISO 8601 format, UTC</i>",
            "dynamicDaylightTimeDisabled": false|true,
            "timeZoneKeyName": "<i>registry key name</i>",
            "timeZoneBias": <i>bias</i>,
            "timeZoneDaylightDayOfWeek": <i>0 for Sunday, etc</i>,
            "timeZoneDaylightBias": <i>daylightBias</i>,
            "timeZoneDaylightDate": "<i>Datetime in ISO 8601 format, UTC</i>",
            "timeZoneDaylightName": "<i>daylight display name</i>",
            "timeZoneStandardDayOfWeek": <i>0 for Sunday, etc</i>,
            "timeZoneStandardBias": <i>standardBias</i>,
            "timeZoneStandardDate": "<i>Datetime in ISO 8601 format, UTC</i>",
            "timeZoneStandardName": "<i>standard display name</i>",
            "lastChange": {
                "time" : "<i>timestamp</i>",
                "state" : "pending|committed|failed",
                "errSubSystem" : "<i>error sub-system</i>",
                "errCode" : <i>error code</i>,
                "errContext" : "<i>error context</i>"
            }
        }
    }
}
</pre>

#### Examples

To configure the device to Pacifict Standard Time, the <i>timeInfo</i> is set to:

<pre>
"desired" : {
    "windows" : {
        "timeInfo": {
            "ntpServer": "time.windows.com",
            "dynamicDaylightTimeDisabled": false,
            "timeZoneKeyName": "Pacific Standard Time",
            "timeZoneBias": 480,
            "timeZoneDaylightBias": -60,
            "timeZoneDaylightDate": "3/2/2016 2:00:00 AM",
            "timeZoneDaylightName": "Pacific Daylight Time",
            "timeZoneStandardBias": 0,
            "timeZoneStandardDate": "11/1/2016 2:00:00 AM",
            "timeZoneStandardName": "Pacific Standard Time",
        }
    }
}
</pre>

The reported settings will looks something like this:
<pre>
"reported" : {
    "windows" : {
        "timeInfo": {
            "ntpServer": "time.windows.com",
            "dynamicDaylightTimeDisabled": false,
            "timeZoneKeyName": "Pacific Standard Time",
            "timeZoneBias": 480,
            "timeZoneDaylightBias": -60,
            "timeZoneDaylightDate": "2016-03-02T02:00:00Z",
            "timeZoneDaylightName": "Pacific Daylight Time",
            "timeZoneStandardBias": 0,
            "timeZoneStandardDate": "2016-11-01T02:00:00Z",
            "timeZoneStandardName": "Pacific Standard Time",
            "localTime": "2017-03-02T01:31:15.0000483Z"
            "lastChange": {
                "time" : "<i>timestamp</i>",
                "state" : "committed"
            }
        }
    }
}
</pre>

### Time Service

The *Time Service* supports [Source Policy](source-policy.md).

#### Device Twin Setting

<pre>
"desired" : {
    "windows": {
        "timeService": {
            "enabled": "yes|no",
            "startup": "auto|manual",
            "started": "yes|no",
            "sourcePriority": "local|remote"
        }
    }
}
</pre>

#### Device Twin Reporting

<pre>
"reported" : {
    "windows": {
        "timeService": {
            "enabled": "yes|no",
            "startup": "auto|manual|n/a",
            "started": "yes|no|n/a",
            "sourcePriority": "local|remote",
            "lastChange": {
                "time" : "<i>timestamp</i>",
                "state" : "<i>status</i>"
            }
        }
    }
}
</pre>

#### .Net API

<pre>
    <b>Namespace</b>:
    Microsoft.Devices.Management
</pre>

<pre>
    public enum ServiceStartup
    {
        Manual,
        Auto
    }

    public enum SettingsPriority
    {
        Unknown,
        Local,
        Remote
    }

    public class TimeServiceState
    {
        public bool enabled;
        public ServiceStartup startup;
        public bool started;
        public SettingsPriority settingsPriority;
    }
</pre>

<pre>
    <b>Class</b>:
    DeviceManagementClient
</pre>

<pre>
    <b>Methods</b>:
    public async Task SetTimeServiceAsync(TimeServiceState desiredState);
    public async Task&lt;TimeServiceState&gt; GetTimeServiceStateAsync();
</pre>

----

[Home Page](../README.md) | [Library Reference](library-reference.md)