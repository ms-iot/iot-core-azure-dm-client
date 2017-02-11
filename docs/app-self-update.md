## Trigger Application Self-Update

The **Trigger Application Self-Update** operation is initiated by the device receiving the `microsoft.management.startAppSelfUpdate` method.

## Input Payload 
Input payload is empty

## Output Payload
The device responds immediately with the following JSON payload:

<pre>
"response" : value (<i>See below</i>)
</pre>

Possible `"response"` values are: 
- `"success"` - The request was accepted and scheduled. The device will attempt check for available update. If any update are found, it will download and install them.
- `"failure"` - The device failed to schedule the update check.

The state of the latest update check attempt is communicated to the back-end via 
reported properties as described in [Device Twin Communication](#define-twin-communication) below.

**Examples:**

Successful response:

```
"response" : "success"
```

## Device Twin Communication

After responding to the method call, the application attempts to check for available updates. The result
of the attempt is recorded in the reported property `"reported.microsoft.management.appUpdate"`, which
is a JSON object with two key/value pairs defined as follows:

<pre>
"reported" : {
    "microsoft" : {
        "management" : {
            "appUpdate" : {
                "lastCheck" : "<i>Datetime in ISO 8601 format, UTC</i>"
                "status" : "checkStarting" <i>or</i> "noUpdates" <i>or</i> "updatesDownloadingAndInstalling" <i>or</i> "installed" <i>or</i> "failed"
            }
        }
    }
}
</pre>

**Examples:**

Update in progress:

```
"reported" : {
    "microsoft" : {
        "management" : {
            "appUpdate" : {
                "lastCheck" : "2017-01-25T13:27:33+04:00",
                "status" : "updatesDownloadingAndInstalling"
            }
        }
    }
}
```

Successful update response:

```
"reported" : {
    "microsoft" : {
        "management" : {
            "appUpdate" : {
                "lastCheck" : "2017-01-25T13:27:33+04:00",
                "status" : "installed"
            }
        }
    }
}
```
