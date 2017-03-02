# Time

The **Time** functionality allows the operator to configure the time zone and the time synchronization server.

## Setting Time Configuration
The format of the `"desired.microsoft.management.timeInfo"` desired property is as follows:

<pre>
    "desired" : {
      "microsoft": {
        "management": {
          "timeInfo": {
            "timeZoneDaylightBias": <i>daylightBias</i>,
            "timeZoneDaylightDate": "<i>Datetime in ISO 8601 format, UTC</i>",
            "timeZoneDaylightName": "<i>daylight display name</i>",
            "timeZoneStandardBias": <i>standardBias</i>,
            "timeZoneStandardDate": "<i>Datetime in ISO 8601 format, UTC</i>",
            "timeZoneStandardName": "<i>standard display name</i>",
            "timeZoneBias": <i>bias</i>,
            "ntpServer": "<i>ntpServer</i>"
          }
        }
      }
    }
</pre>

## Reporting Time Configuration
The format of the `"reported.microsoft.management.timeInfo"` desired property is as follows:

<pre>
    "reported" : {
      "microsoft": {
        "management": {
          "timeInfo": {
            "Tag": 30,
            "Status": <i>status</i>,
            "timeZoneDaylightBias": <i>daylightBias</i>,
            "timeZoneDaylightDate": "<i>Datetime in ISO 8601 format, UTC</i>",
            "timeZoneDaylightName": "<i>daylight display name</i>",
            "timeZoneStandardBias": <i>standardBias</i>,
            "timeZoneStandardDate": "<i>Datetime in ISO 8601 format, UTC</i>",
            "timeZoneStandardName": "<i>standard display name</i>",
            "timeZoneBias": <i>bias</i>,
            "ntpServer": "<i>ntpServer</i>",
            "localTime": "<i>Datetime in ISO 8601 format, UTC</i>"
          }
        }
      }
    }
</pre>

**Examples**

To configure the device to Pacifict Standard Time, the <i>timeInfo</i> is set to:

```
"desired" : {
    "microsoft" : {
        "management" : {
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
}
```

The reported settings will looks something like this:
```
"reported" : {
    "microsoft" : {
        "management" : {
          "timeInfo": {
            "Tag": 30,
            "Status": 0,
            "timeZoneDaylightBias": -60,
            "timeZoneDaylightDate": "2016-03-02T02:00:00Z",
            "timeZoneDaylightName": "Pacific Daylight Time",
            "timeZoneStandardBias": 0,
            "timeZoneStandardDate": "2016-11-01T02:00:00Z",
            "timeZoneStandardName": "Pacific Standard Time",
            "timeZoneBias": 480,
            "ntpServer": "time.windows.com",
            "localTime": "2017-03-02T01:31:15.0000483Z"
          }
        }
    }
}
```
