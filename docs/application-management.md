# Application Management

The following functionality is implemented:

- Configuring Application State on the Device.
- Triggering Self-Update for the Device Management Store application.

## Configuring Application State on the Device

Device Management client supports declarative app management with Azure IoT Hub device twin. The `"desired.microsoft.management.apps"` property represents the desired state of the apps installed on the device. The DM client compares that state with the actual state (by performing an inventory of the apps installed on the device) and makes up the difference.

### Desired Properties

The `"desired.microsoft.management.apps"` property is defined like this:

<pre>
"desired" : {
    "microsoft" : {
        "management" : {
            "apps" : {
                "<i>packageFamilyId</i>" : {
                    "pkgFamilyName" : "<i>packageFamilyName</i>"
                    "version" : "<i>see below</i>",
                    "appxSource" : "<i>see below</i>",
                    "depsSources" : "<i>see below</i>",
                    "certSource" : "<i>see below</i>",
                    "certStore" : "<i>see below</i>",
                }
                <i>[,...]</i>
            }
        }
    }
}
</pre>

#### Details

The `"desired.microsoft.management.apps"` property has a list of apps that are reported (though not necessarily installed) on the device. 

Each app is identified by its `"packageFamilyId"` - which is its package family name in a form that can be used as a json property name (for example, you can replace illegal characters with underscores).

- If the value is `null`, the state of the app will no longer be reported in the reported properties. This is mainly used to clean up the reported state.
- Otherwise, if the value is not `null`, it is an object that has the following properties:
    - `"pkgFamilyName"`: the package family name of the target application.
    - `"version"`: this property indicate the desired state of that application on the device. It can take one of several forms:
        - `Major.Minor.Build.Revision`: This indicates that the specified version is to be installed and reported.
        - `"?"` : This indicates that the status of this application is to be reported.
        - `"not installed"` : This indicates that the specified application should not be installed on the device. If it is already installed, the DM client will uninstall it.
    - `"appxSource": "container\appPackage.appx"`
       - The appx package file name in the Azure blob storage. The access to the blob storage is performed as described [here](blob-storage.md). The value of this property cannot be empty or null if the app is to be installed on the device.
    - `"depsSources": "container\dep0.appx;container\dep1.appx"`
       - The semi-colon separated relative paths (in Azure Storage) of dependencies.
    - `"certSource" : "container\certificate.cer"`
       - The certificate file (in Azure Storage) to be installed.
    - `"certStore"`: "./Device/Vendor/MSFT/<i>store</i>"
       - The full certificate CSP path to have the certificate installed to - for example: `"./Device/Vendor/MSFT/RootCATrustedCertificates/TrustedPeople"`.

#### Update Frequency

The DM client performs the update check according to the [Store Update Configuration](store-update-config.md). The update check is not performed each time a value in `"desired.microsoft.management.apps"` changes.

### Reported Properties

The DM client maintains the inventory of installed apps in the `"reported.microsoft.management.apps"` property. Only the apps listed in the `"desired.microsoft.management.apps"` property with non-`null` value are reported.

#### Successful Case

If the DM client is able to bring the actual state in compliance with the desired state, the format of the `"reported.microsoft.management.apps"` property is as follows:

<pre>
"reported" : {
    "microsoft" : {
        "management" : {
            "apps" : {
                "<i>packageFamilyId</i>" : {
                    "pkgFamilyName" : "<i>see below</i>",
                    "version" : "<i>see below</i>",
                    "installDate" : "<i>Datetime in ISO 8601 format, UTC</i>"
                    "errorCode" : "<i>see below</i>",
                    "errorMessage" : "<i>see below</i>",
                }
                <i>[,...]</i>
            }
        }
    }
}
</pre>

The reported `"version"` value can take one of several forms:

- Value formatted as <b>Major.Minor.Build</b> is reported for a currently installed app.
- Value `"not installled"` is reported for apps that are reported but are not installed.

The reported `"installDate"` property is not present if the value of `"version"` is `"not installed"`.

Only the apps that are reported in `"desired.microsoft.management.apps"` are listed in `"reported.microsoft.management.apps"`. 

#### Unsuccessful Case

If the DM client is not able to bring the actual state in compliance with the desired state, the failure is reported in the `"reported.microsoft.management.apps"` property for each app. For example, this happens if the DM client is not able to install a requested version of the app, or when the app could not be uninstalled. 

The format of the unsuccessful is as follows:

<pre>
"reported" : {
    "microsoft" : {
        "management" : {
            "apps" : {
                "<i>packageFamilyId</i>" : {
                    ...
                    "errorCode" : "<i>see below</i>"
                    "errorMessage" : "<i>see below</i>"
                }
                <i>[,...]</i>
            }
        }
    }
}
</pre>

The value of the `"errorMessage"` property is a string in implementation-defined format (for example, it could be an error message, the exception text or a call stack).

Note that the `"errorCode"` and `"errorMessage"` property can coexist with the `"version"` property. For example, if an app cannot be updated, the reported properties will still have the actual version. 
However, if the app cannot be installed at all, the `"version"` will be set to `"not installed"`.

`"errorCode"` may have one of the following values:

|    code     |    meaning                             |
|:-----------:|----------------------------------------|
| 0xA0000000  | Invalid desired version                |
| 0xA0000001  | Invalid desired package family id      |
| 0xA0000002  | Invalid desired appx source            |
| 0xA0000003  | Invalid appx operation                 |
| 0xA0000004  | Appx package has been installed successfully, but the version does not match the desired version, and is the same as the old version. |
| 0xA0000005  | Appx package has been installed successfully, but the version does not match the desired version. |
| 0x8xxxxxxx  | OS error - check Windows documentation |

### Examples

#### Example 1

The operator wishes to ensure that the Toaster app (with package family name `23983CETAthensQuality.IoTToasterSample`) and the GardenSprinkler app (`GardenSprinkler_kay8908908`) are installed on the device. Additionally, the operator wants to have the DogFeeder app (`DogFeeder_80615fge`) uninstalled from the device:

<pre>
"desired" : {
    "microsoft" : {
        "management" : {
            "apps" : { 
                "23983CETAthensQuality_IoTToasterSample" : {
                    "pkgFamilyName" : "23983CETAthensQuality.IoTToasterSample"
                    "version" : "1.1.0.0",
                    "appxSource" : "apps\\IoTToasterSample.1.1.0.0.appx"
                    "depsSources": "apps\\Microsoft.NET.CoreRuntime.1.1.appx;apps\\Microsoft.VCLibs.x86.Debug.14.00.appx",
                    "certSource": "apps\\IoTToasterSample.cer",
                    "certStore": "./Device/Vendor/MSFT/RootCATrustedCertificates/TrustedPeople"
                },
                "GardenSprinkler_kay8908908" : {
                    "pkgFamilyName" : "GardenSprinkler.kay8908908"
                    "version" : "2.0.0.0",
                },
                "DogFeeder_80615fge" : {
                    "pkgFamilyName" : "DogFeeder.80615fge"
                    "version" : "not installed",
                }
            }
        }
    }
}
</pre>

Note that for the `"GardenSprinkler_kay8908908"` application no sources were specified. This indicates that the target action is to update from the store if the currently installed version is older than 2.0.0.0.

The client determines the required set of actions, performs them and updates the `"reported.microsoft.management.apps"` property as follows:

<pre>
"reported" : {
    "microsoft" : {
        "management" : {
            "apps" : { 
                "23983CETAthensQuality.IoTToasterSample" : {
                    "pkgFamilyName" : "23983CETAthensQuality.IoTToasterSample"
                    "version" : "1.1.0.0",
                    "installDate" : "2017-02-25T09:00:00+00:00"
                },
                "GardenSprinkler_kay8908908" : {
                    "pkgFamilyName" : "GardenSprinkler.kay8908908"
                    "version" : "2.0.0.0",
                    "installDate" : "2017-02-25T09:15:00+00:00"
                },
                "DogFeeder_80615fge" : {
                    "pkgFamilyName" : "DogFeeder.80615fge"
                    "version" : "not installed",
                    "installDate" : null
                }
            }
        }
    }
}
</pre>

#### Example 2

In the following example, the operator wishes to stop tracking the state of the DogFeeder app, which has been decommissioned. This is expressed as follows:

<pre>
"desired" : {
    "microsoft" : {
        "management" : {
            "apps" : { 
                "DogFeeder_80615fge" : null
            }
        }
    }
}
</pre>

After this, the reported properties look like this:

<pre>
"reported" : {
    "microsoft" : {
        "management" : {
            "apps" : { 
                "23983CETAthensQuality.IoTToasterSample" : {
                    "pkgFamilyName" : "23983CETAthensQuality.IoTToasterSample"
                    "version" : "1.1.0.0",
                    "installDate" : "2017-02-25T09:00:00+00:00"
                },
                "GardenSprinkler_kay8908908" : {
                    "pkgFamilyName" : "GardenSprinkler.kay8908908"
                    "version" : "2.0.0.0",
                    "installDate" : "2017-02-25T09:15:00+00:00"
                },
                "DogFeeder_80615fge" : null
            }
        }
    }
}
</pre>

#### Example 3

Next, the operator wishes to upgrade the Toaster app to version 2.0.0.0 using the appx file stored in the Azure storage located at `AppContainer/Toaster.appx`:

<pre>
"desired" : {
    "microsoft" : {
        "management" : {
            "apps" : { 
                "23983CETAthensQuality_IoTToasterSample" : {
                    "pkgFamilyName" : "23983CETAthensQuality.IoTToasterSample",
                    "appxSource" : "AppContainer/Toaster.appx",
                    "version" : "2.0.0.0"
                    "appxSource" : "apps\\IoTToasterSample.2.0.0.0.appx"
                    "depsSources": "apps\\Microsoft.NET.CoreRuntime.1.1.appx;apps\\Microsoft.VCLibs.x86.Debug.14.00.appx",
                    "certSource": "apps\\IoTToasterSample.cer",
                    "certStore": "./Device/Vendor/MSFT/RootCATrustedCertificates/TrustedPeople"
                },
                ...
            }
        }
    }
}
</pre>

If the installation fails (for example, if Toaster.appx specified in the `"source"` property is not present or does not contain app version 2.0.0.0), the error is reported as follows:

<pre>
"reported" : {
    "microsoft" : {
        "management" : {
            "apps" : { 
                "23983CETAthensQuality_IoTToasterSample" : {
                    "pkgFamilyName" : "23983CETAthensQuality.IoTToasterSample",
                    "version" : "1.5.0.0",
                    "installDate" : "2017-02-25T09:15:00+00:00"
                    "errorCode" : "<i>OS error</i>",
                    "errorMessage" : "Cannot install app; The app version in Toaster.appx is 1.5.0.0; desired version is 2.0.0.0"
                },
                ...             
            }
        }
    }
}
</pre>

#### Example 4

When an operator needs to determine the version of an app (usually because it was not reported), it can be accomplished by specifying the "?" in the version value. For example, to determine the HumiditySensor app (with a known package familiy name `HumiditySensor_76590kat`) is present, the desired propety looks like this:

<pre>
"desired" : {
    "microsoft" : {
        "management" : {
            "apps" : { 
                "HumiditySensor_76590kat" : {
                    "pkgFamilyName" : "HumiditySensor_76590kat",
                    "version" : "?"
                },
                ...
            }
        }
    }
}
</pre>

The state of the HumiditySensor app is then reported like this:

<pre>
"reported" : {
    "microsoft" : {
        "management" : {
            "apps" : { 
                "HumiditySensor_76590kat" : {
                    "pkgFamilyName" : "HumiditySensor_76590kat",
                    "version" : "5.1.0.0",
                    "installDate" : "2016-04-21T05:00:00+00:00"
                },
                ...
            }
        }
    }
}
</pre>

## Triggering Self-Update for the Device Management Store Application 

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

<pre>
"response" : "success"
</pre>

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

<pre>
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
</pre>

Successful update response:

<pre>
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
</pre>
