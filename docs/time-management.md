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
            "timeZoneDaylightBias": <i>daylightBias</i>,
            "timeZoneDaylightDate": "<i>Datetime in ISO 8601 format, UTC</i>",
            "timeZoneDaylightName": "<i>daylight display name</i>",
            "timeZoneDaylightDayOfWeek": <i>0 for Sunday, etc</i>,
            "timeZoneStandardBias": <i>standardBias</i>,
            "timeZoneStandardDate": "<i>Datetime in ISO 8601 format, UTC</i>",
            "timeZoneStandardName": "<i>standard display name</i>",
            "timeZoneStandardDayOfWeek": <i>0 for Sunday, etc</i>,
            "timeZoneBias": <i>bias</i>,
            "ntpServer": "<i>ntpServer</i>"
            }
        }
    }
}
</pre>

#### Reporting
The format of the `"reported.microsoft.management.timeInfo"` desired property is as follows:

<pre>
"reported" : {
    "windows": {
        "timeInfo": {
            "timeZoneDaylightDayOfWeek": <i>0 for Sunday, etc</i>,
            "timeZoneDaylightBias": <i>daylightBias</i>,
            "timeZoneDaylightDate": "<i>Datetime in ISO 8601 format, UTC</i>",
            "timeZoneDaylightName": "<i>daylight display name</i>",
            "timeZoneStandardDayOfWeek": <i>0 for Sunday, etc</i>,
            "timeZoneStandardBias": <i>standardBias</i>,
            "timeZoneStandardDate": "<i>Datetime in ISO 8601 format, UTC</i>",
            "timeZoneStandardName": "<i>standard display name</i>",
            "timeZoneBias": <i>bias</i>,
            "ntpServer": "<i>ntpServer</i>",
            "localTime": "<i>Datetime in ISO 8601 format, UTC</i>"
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
            "timeZoneDaylightBias": -60,
            "timeZoneDaylightDate": "3/2/2016 2:00:00 AM",
            "timeZoneDaylightName": "Pacific Daylight Time",
            "timeZoneStandardBias": 0,
            "timeZoneStandardDate": "11/1/2016 2:00:00 AM",
            "timeZoneStandardName": "Pacific Standard Time",
            "timeZoneBias": 480,
            "ntpServer": "time.windows.com"
        }
    }
}
</pre>

The reported settings will looks something like this:
<pre>
"reported" : {
    "windows" : {
        "timeInfo": {
            "timeZoneDaylightBias": -60,
            "timeZoneDaylightDate": "2016-03-02T02:00:00Z",
            "timeZoneDaylightName": "Pacific Daylight Time",
            "timeZoneStandardBias": 0,
            "timeZoneStandardDate": "2016-11-01T02:00:00Z",
            "timeZoneStandardName": "Pacific Standard Time",
            "timeZoneBias": 480,
            "ntpServer": "time.windows.com",
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

The desired state of the *Time Service* can be set:

- Remotely through the Device Twin.
- Locally through a .Net API interface. This interface can be called by the hosting UWP application.

Both, the remote configuration and the local configuration, can co-exist on the same device - however, only one of them can be applied. To specify which one, a priority has to be set. The priority can be set by either interface.

For example, consider the following:

- The administrator wants all devices to have the Time Service running.
- The administrator wants to give the option to the application user (on the device) to turn off the Time Service.
- The application exposes a way to turn on and off the Time Service.

For the above scenario, 

- The administrator will set the device twin properties to have the Time Service started and set the priority to `local`.
- The application can then call the .Net API (see below), and start/stop the service.

Should the administrator decided to take ownership, and override the application settings, the administrator can set the priority to `remote`, and then apply the desired settings.

Note that both the device twin and the .Net API can control the priority property - which gives them equal rights.

When reporting, it is the current state of the service that is reported - regardless of whether it is configured using the local or the remote settings.

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
"desired" : {
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