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
            "profiles": {
              "WifiProfile1": { 
                "profile": "fileName01.xml",
                "disableInternetConnectivityChecks": "true",
              },
              "WifiProfile2": { 
                "profile": "fileName02.xml",
                "disableInternetConnectivityChecks": "false",
              }
            },
            "active": "WifiProfile1"
          }
        }
      }
    }
</pre>

## List Installed Certificates
The DM client reports the hashes of installed certificates under the pre-defined set of CSP paths. Each set of hashes will appear under the json property name corresponding to the CSPs path as described in the mapping above.

<pre>
    "reported": {
      "microsoft": {
        "management": {
          "wifi": {
            "profiles": [
              "WifiProfile1",
              "WifiProfile2"
            ],
            "active": "WifiProfile1"
          }
        }
      }
    }
</pre>

## Retrieve Wifi Details
To get more details about any of the installed certificates, the request can be initiated by calling the asynchronous `microsoft.management.getProfileDetails` method.
The method will schedule a job on the device to capture the certificate details in a json file and upload it to the specified blob in Azure Storage.
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
- `"accepted"` - The reboot request was accepted. The device will retrieve the certificate details and upload it to the Azure Storage specified in the input parameters.
- `"rejected"` - The device rejected the request.

`"reason"` is used to communicate why an App Install request was rejected if possible.

### Uploaded File Format
<pre>
{
    <TBD>
}
</pre>

## Examples:

### Install Certificates

If the operator wants to install a new wifi profile (home-wifi.xml), the following steps should be followed:
- Upload the certificate file to the default Azure blob storage. Let's assume the profile name is 'HomeWifi'.
- Set the desired properties to:
<pre>
    "desired": {
      "microsoft": {
        "management": {
          "wifi": {
            "profiles": {
              "HomeWifi": { 
                "profile": "home-wifi.xml"
              }
            },
            "activeProfile": "HomeWifi"
          }
        }
      }
    }
</pre>
