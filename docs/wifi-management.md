# Wifi Management

The **Wifi Management** functionality allows the operator to perform the following tasks:
- Install a wifi profile (from blob storage).
- Uninstall a wifi profile.
- List installed wifi profiles.
- Get detailed information about an installed wifi profile.

## Install/Uninstall A Wifi Profile
The operator can specify the list of desired wifi profiles to be installed. When the device receives the desired state, it compares it to the device current state and:
- If a wifi profile exists in the desired list but is not installed on the device, it is installed.
- If a wifi profile is installed on the device, but is not present in the desired list, it is uninstalled.

<pre>
    "desired": {
      "microsoft": {
        "management": {
          "wifi": {
            "applyProperties": {
              "WifiProfile1": {
                "profile": "container/fileName01.xml",
                "disableInternetConnectivityChecks": "true",
              },
              "WifiProfile2": {
                "profile": "container/fileName02.xml",
               "disableInternetConnectivityChecks": "false",
              },
              "activeProfile": "WifiProfile1"
            }
          }
        }
      }
    }
</pre>

## List Installed Wifi Profiles
The DM client reports the names of installed wifi profiles. 

<pre>
    "reported": {
      "microsoft": {
        "management": {
          "wifi": {
            "WifiProfile1": "active",
            "WifiProfile2": ""
          }
        }
      }
    }
</pre>

## Retrieve Wifi Details
To get more details about any of the installed wifi profiles, the request can be initiated by calling the asynchronous `microsoft.management.getProfileDetails` method.
The method will schedule a job on the device to capture the wifi profile details in a json file and upload it to the specified blob in Azure Storage.
The method returns immediately and indicates that it has accepted or rejected the job.

### Input Payload 
```
{
    "profileName" : "WifiProfile1",
    "connectionString" : "connectionStrng",
    "containerName" : "containerName",
    "output" : "blobFileName"
}
````

## Output Payload
The device responds immediately with the following JSON payload:

```
{
    "response" : value (See below)
    "reason" : value (See below)
}
```

Possible `"response"` values are: 
- `"accepted"` - The reboot request was accepted. The device will retrieve the profile details and upload it to the Azure Storage specified in the input parameters.
- `"rejected"` - The device rejected the request.

`"reason"` is used to communicate why an App Install request was rejected if possible.

### Uploaded File Format
<pre>
{
    <TBD>
}
</pre>

## Examples:

### Install Profile

If the operator wants to install a new wifi profile (home-wifi.xml), the following steps should be followed:
- Upload the wifi profile xml file to the default Azure blob storage. Let's assume the profile name is 'HomeWifi'.
- Set the desired properties to:
<pre>
    "desired": {
      "microsoft": {
        "management": {
          "wifi": {
            "applyProperties": {
              "HomeWifi": {
                "profile": "container/home-wifi.xml"
              },
              "activeProfile": "HomeWifi"
            }
          }
        }
      }
    }
</pre>

### Uninstall Profile

If the operator wants to uninstall a wifi profile (home-wifi.xml), the following steps should be followed:
- Set the desired properties to:
<pre>
    "desired": {
      "microsoft": {
        "management": {
          "wifi": {
            "applyProperties": {
              "HomeWifi": "uninstall"
            }
          }
        }
      }
    }
</pre>

### Not interested in configuring or reporting Wifi properties
- Set the desired properties to:
<pre>
    "desired": {
      "microsoft": {
        "management": {
          "wifi": "no-no"
        }
      }
    }
</pre>
- Equivalent, but more verbose:
<pre>
    "desired": {
      "microsoft": {
        "management": {
          "wifi": {
            "applyProperties": "no"
            "reportProperties": "no"
          }
        }
      }
    }
</pre>

### Only interested in reporting Wifi properties

- Set the desired properties to:
<pre>
    "desired": {
      "microsoft": {
        "management": {
          "wifi": "no-yes"
        }
      }
    }
</pre>
- Equivalent, but more verbose:
<pre>
    "desired": {
      "microsoft": {
        "management": {
          "wifi": {
            "applyProperties": "no"
            "reportProperties": "yes"
          }
        }
      }
    }
</pre>
