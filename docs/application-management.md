# Application Management

The following functionality is implemented:
- Installing a Store Application from Azure Storage
- Trigger Application Self-Update

## Installing a Store Application

The **Install App** operation is initiated by the device receiving the `microsoft.management.appInstall` method. The method starts the process of downloading and installing an Appx package along with its dependencies. The application is hosted in an Azure blob, access to which is provided to the client via the connection string, as specified below.

### Input Payload 
Input payload contains Json according to this format:

```
{
  "PackageFamilyName" : "MyCoolApp_1abcde2f3ghij",
  "Appx": {
    "ConnectionString" : "DefaultEndpointsProtocol=https;AccountName=myaccount;AccountKey=blah",
    "ContainerName" : "MyAppContainer",
    "BlobName" : "MyCoolApp.appx"
  },
  "Dependencies": [
    {
      "ConnectionString" : "DefaultEndpointsProtocol=https;AccountName=myaccount;AccountKey=blah",
      "ContainerName" : "MyDependencyContainer",
      "BlobName" : "MyCoolAppDependency.appx"
    }
  ]
}
```

### Output Payload
The device responds immediately with the following JSON payload:

```
{
    "response" : value (See below)
    "reason" : value (See below)
}
```


Possible `"response"` values are: 
- `"accepted"` - The parameters sent were parsed and the App Install request was accepted.
- `"rejected"` - The parameters sent failed to parsed and the App Install request was rejected.  See the reason field for explanation.

`"reason"` is used to communicate why an App Install request was rejected if possible.

## Triggering Store Application Self-Update

The **Trigger Application Self-Update** operation is initiated by the device receiving the `microsoft.management.startAppSelfUpdate` method.

### Input Payload 
Input payload is empty

### Output Payload
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

### Device Twin Communication

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
