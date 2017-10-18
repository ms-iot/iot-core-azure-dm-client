# Wifi Management

The **Wifi Management** functionality allows the operator to perform the following tasks:
- Install a wifi profile (from blob storage).
- Uninstall a wifi profile.
- List installed wifi profiles.
- Get detailed information about an installed wifi profile.

## Install/Uninstall A Wifi Profile
The operator can specify the list of desired wifi profiles to be installed or uninstalled. When the device receives the desired state, it compares it to the device current state and:
- If a wifi profile exists in the desired list but is not installed on the device, it is installed with the provided properties.
- To uninstall a wifi profile, specify the profile name and set its value to "uninstall". This will have effect only if that wifi profile is already present on the machine - if not, no action will be taken.

<pre>
    "desired": {
      "windows": {
        "wifi": &lt;<i>see below</i>&gt;
      }
    }
</pre>

- ```"wifi"``` can be set to one of the following:
    - ```"no-apply-no-report"``` : This means no desired state, and no reported state will be stored in the device twin.
  - ```"no-apply-yes-report"```: This means no desired state will be stored in the device twin, but reported state will.
  - A json object of the following format:

<pre>
    "desired": {
      "windows": {
        "wifi": {
          "applyProperties": {
            "WifiProfile1": {
            "profile": "<i>see below</i>"
            },
            "WifiProfile2": "uninstall"
        },
        "reportProperties" : &lt;<i>see below</i>&gt;
        }
      }
    }
</pre>

- `"profile": "container\profile.xml"`
  - The wifi profile file name in the Azure blob storage. The access to the blob storage is performed as described [here](external-storage.md). The value of this property cannot be empty or null if the profile is to be installed on the device.  The file must follow the [WLAN_profile Schema](http://go.microsoft.com/fwlink/p/?LinkId=325608) on MSDN.
- The ```"reportProperties"``` can be set to one of the following values:
  - ```"yes"```: tells the DM client to report the Windows Update Policy state of the device.
  - ```"no"```: tells the DM client to not report the Wifi section in the reported properties. This can be useful to free some room in the Device Twin.

## List Installed Wifi Profiles
The device current state of the **Wifi** can be inspected through the ```"wifi"``` node in the reported properties section as follows:

<pre>
    "reported": {
      "windows": {
        "wifi": {
          "WifiProfile1": "",
          "WifiProfile2": ""
        }
      }
    }
</pre>

## Retrieve Wifi Details
To get more details about any of the installed wifi profiles, the request can be initiated by calling the asynchronous `microsoft.management.getWifiDetails` method.
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
The XML that describes the network configuration and follows the [WLAN_profile Schema](http://go.microsoft.com/fwlink/p/?LinkId=325608) on MSDN.

### Install Profile

If the operator wants to install a new wifi profile (home-wifi.xml), the following steps should be followed:
- Upload the wifi profile xml file to Azure blob storage. The access to the blob storage is performed as described [here](blob-storage.md). To ensure uniqueness and optimize downloading, the name specified here (in this example 'HomeWifi'), must be the SSID\Name specified in the profile XML.  The XML must follow the [WLAN_profile Schema](http://go.microsoft.com/fwlink/p/?LinkId=325608) on MSDN.
- Set the desired properties to:
<pre>
    "desired": {
      "windows": {
        "wifi": {
          "applyProperties": {
            "HomeWifi": {
            "profile": "container/home-wifi.xml"
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
      "windows": {
        "wifi": {
          "applyProperties": {
            "HomeWifi": "uninstall"
          }
        }
      }
    }
</pre>

### Not interested in configuring or reporting Wifi properties
- Set the desired properties to:
<pre>
    "desired": {
      "windows": {
        "wifi": "no-apply-no-report"
      }
    }
</pre>
- Equivalent, but more verbose:
<pre>
    "desired": {
      "windows": {
        "wifi": {
          "applyProperties": "no"
          "reportProperties": "no"
        }
      }
    }
</pre>

### Only interested in reporting Wifi properties

- Set the desired properties to:
<pre>
    "desired": {
      "windows": {
        "wifi": "no-apply-yes-report"
      }
    }
</pre>
- Equivalent, but more verbose:
<pre>
    "desired": {
      "windows": {
        "wifi": {
          "applyProperties": "no"
          "reportProperties": "yes"
        }
      }
    }
</pre>

----

[Home Page](../README.md) | [Library Reference](library-reference.md)