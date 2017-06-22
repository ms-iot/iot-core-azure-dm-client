# Application Management

The following functionality is implemented:

- Managing application state and reporting
  - Query which applications are install/uninstalled.
  - Install new applications or uninstall existing ones.
  - Mark applications to be the foreground application or background tasks.
- Starting and stopping applications on the device.
- Triggering Self-Update for the Device Management Store application.

## Managing Application State and Reporting

Device Management client supports declarative app management with Azure IoT Hub device twin. The `"desired.microsoft.management.apps"` property represents the desired state of the apps installed on the device as well as how to report the state to the device twin.
The DM client compares that state with the actual state (by performing an inventory of the apps installed on the device) and makes up the difference.

The `"desired.microsoft.management.apps"` may contain two types of nodes:

- `"packageFamilyId"` nodes describe the desired state of a given application.
- `"?"` node describes whether the DM client should report store and/or non-store installed applications to the device twin or not.

### Desired Properties - Application Desired State

The `"desired.microsoft.management.apps"` property is defined like this:

<pre>
"desired" : {
    "microsoft" : {
        "management" : {
            "apps" : {
                "<i>packageFamilyId</i>" : {
                    "pkgFamilyName" : "<i>packageFamilyName</i>",
                    "version" : "<i>see below</i>",
                    "startUp" : "none|foreground|background",
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
         - If the desired version is the same as the one installed, no installs/uninstalls will take place. However, other applicable settings (like startUp) will be applied.
         - If the desired version is older than the one installed, the installed application will be uninstalled and the specified one will be installed. Applicable settings will be processed afterwards.
         - If the desired version is new than the one install, the specified one will installed on top of the existing one. Applicable settings will be processed.
         - <b>Note</b>: Installing a new application from the store is not supported.
         - <b>Note</b>: Update applications other than the DM application from the store is not supported.
       - `"?"` : This indicates that the status of this application is to be reported.
       - `"not installed"` : This indicates that the specified application should not be installed on the device. If it is already installed, the DM client will uninstall it.
    - `"startUp"`: the start-up designation of this application. Possible values are:
       - `"none"`: this application is neither the foreground application or a background task.
       - `"foreground"`: this application is the foreground application. Only one application can have `"startUp"` set to this value. When switching from foreground application to another, both application must appear in the same transaction. The order is not relevant though as the DM client re-orders them.
       - `"background"`: this application is a background task. Any number of applications can be set as background tasks.
    - `"appxSource": "container\appPackage.appx"`
       - The appx package file name in the Azure blob storage. The access to the blob storage is performed as described [here](external-storage.md). The value of this property cannot be empty or null if the app is to be installed on the device.
    - `"depsSources": "container\dep0.appx;container\dep1.appx"`
       - The semi-colon separated relative paths (in Azure Storage) of dependencies.
    - `"certSource" : "container\certificate.cer"`
       - The certificate file (in Azure Storage) to be installed.
    - `"certStore"`: "./Device/Vendor/MSFT/<i>store</i>"
       - The full certificate CSP path to have the certificate installed to - for example: `"./Device/Vendor/MSFT/RootCATrustedCertificates/TrustedPeople"`.

### Desired Properties - Reporting Store/Non-Store Applications

By default, only the application explicitly specified by the packageFamilyId in the desired sections will have their state reported. To report other applications that are already installed, use the `"?"` as described below.

<pre>
"desired" : {
    "microsoft" : {
        "management" : {
            "apps" : {
                "?" : {
                    "store" : "<i>true|false</i>"
                    "nonStore" : "<i>true|false</i>",
                }
            }
        }
    }
}
</pre>

#### Details

Note that the `"?"` can be combined with `"packageFamilyId"` elements. The reported set will be a union of applications identified by the `"packageFamilyId"`s and the subset specified in the `"?"` object.

### Store Update Frequency

The DM client performs the update check according to the [Store Update Configuration](store-update-config.md). The update check is not performed each time a value in `"desired.microsoft.management.apps"` changes.

### Reported Properties

The DM client maintains the inventory of installed apps in the `"reported.microsoft.management.apps"` property. Only the apps listed in the `"desired.microsoft.management.apps"` property with non-`null` value are reported.

<b>Note:</b> The contents of the `"reported.microsoft.management.apps"` will be set to `"refreshing"` to indicate that the sub-tree is being reconstructed. This allows the removal of stale elements in the list. Should you query the node and find its value set to `"refreshing"`, wait for a few seconds and try again until it has the list of applications.

#### Example of A Successful Case

If the DM client is able to bring the actual state in compliance with the desired state, the format of the `"reported.microsoft.management.apps"` property is as follows:

<pre>
"reported" : {
    "microsoft" : {
        "management" : {
            "apps" : {
                "<i>packageFamilyId</i>" : {
                    "pkgFamilyName" : "<i>see below</i>",
                    "version" : "<i>see below</i>",
                    "startUp" : "none|foreground|background",
                    "installDate" : "<i>Datetime in ISO 8601 format, UTC</i>"
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

#### Example of An Unsuccessful Case

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
| 0xA0000006  | The desired state designates two applications to be the foreground application. |
| 0xA0000007  | An application scheduled for uninstall is also being set to be the foreground application. |
| 0x8xxxxxxx  | OS error - check Windows documentation |

### Examples

#### Example 1

The operator wishes to ensure that the Toaster app (with package family name `23983CETAthensQuality.IoTToasterSample`) and the GardenSprinkler app (`GardenSprinkler_kay8908908`) are installed on the device. Additionally, the operator wants to have the DogFeeder app (`DogFeeder_80615fge`) uninstalled from the device and all installed store applications reported:

<pre>
"desired" : {
    "microsoft" : {
        "management" : {
            "apps" : { 
                "23983CETAthensQuality_IoTToasterSample" : {
                    "pkgFamilyName" : "23983CETAthensQuality.IoTToasterSample"
                    "version" : "1.1.0.0",
                    "startUp" : "none",
                    "appxSource" : "apps\\IoTToasterSample.1.1.0.0.appx"
                    "depsSources": "apps\\Microsoft.NET.CoreRuntime.1.1.appx;apps\\Microsoft.VCLibs.x86.Debug.14.00.appx",
                    "certSource": "apps\\IoTToasterSample.cer",
                    "certStore": "./Device/Vendor/MSFT/RootCATrustedCertificates/TrustedPeople"
                },
                "GardenSprinkler_kay8908908" : {
                    "pkgFamilyName" : "GardenSprinkler.kay8908908"
                    "startUp" : "foreground",
                    "version" : "2.0.0.0",
                },
                "DogFeeder_80615fge" : {
                    "pkgFamilyName" : "DogFeeder.80615fge"
                    "startUp" : "background",
                    "version" : "not installed",
                },
                "?" {
                    "store": true,
                    "nonStore": false
                }
            }
        }
    }
}
</pre>

Note that for the `"GardenSprinkler_kay8908908"` application no sources were specified. This indicates that the target action is to update from the store if the currently installed version is older than 2.0.0.0.
Also note that BirdFeeder_80615fge is included in the reported list because the user has `"?"` in the desired list.

The client determines the required set of actions, performs them and updates the `"reported.microsoft.management.apps"` property as follows:

<pre>
"reported" : {
    "microsoft" : {
        "management" : {
            "apps" : { 
                "23983CETAthensQuality.IoTToasterSample" : {
                    "pkgFamilyName" : "23983CETAthensQuality.IoTToasterSample"
                    "version" : "1.1.0.0",
                    "startUp" : "none",
                    "installDate" : "2017-02-25T09:00:00+00:00"
                },
                "GardenSprinkler_kay8908908" : {
                    "pkgFamilyName" : "GardenSprinkler.kay8908908"
                    "version" : "2.0.0.0",
                    "startUp" : "foreground",
                    "installDate" : "2017-02-25T09:15:00+00:00"
                },
                "DogFeeder_80615fge" : {
                    "pkgFamilyName" : "DogFeeder.80615fge"
                    "version" : "not installed",
                    "startUp" : "background",
                    "installDate" : null
                },
                "BirdFeeder_80615fge" : {
                    "pkgFamilyName" : "BirdFeeder.80615fge"
                    "version" : "1.0.0.0",
                    "startUp" : "none",
                    "installDate" : null
                },
            }
        }
    }
}
</pre>

#### Example 2

In the following example, the operator wishes to stop tracking the state of the DogFeeder app, which has been decommissioned but still wants to list all the store applications. This is expressed as follows:

<pre>
"desired" : {
    "microsoft" : {
        "management" : {
            "apps" : { 
                "DogFeeder_80615fge" : null,
                "?" {
                    "store": true,
                    "nonStore": false
                }
            }
        }
    }
}
</pre>

Given that the DogFeeder application is a nonStore application, after setting the above desired properties, the reported properties look like this:

<pre>
"reported" : {
    "microsoft" : {
        "management" : {
            "apps" : { 
                "23983CETAthensQuality.IoTToasterSample" : {
                    "pkgFamilyName" : "23983CETAthensQuality.IoTToasterSample"
                    "version" : "1.1.0.0",
                    "startUp" : "none",
                    "installDate" : "2017-02-25T09:00:00+00:00"
                },
                "GardenSprinkler_kay8908908" : {
                    "pkgFamilyName" : "GardenSprinkler.kay8908908"
                    "version" : "2.0.0.0",
                    "startUp" : "foreground",
                    "installDate" : "2017-02-25T09:15:00+00:00"
                },
                "DogFeeder_80615fge" : null,
                "BirdFeeder_80615fge" : {
                    "pkgFamilyName" : "BirdFeeder.80615fge"
                    "version" : "1.0.0.0",
                    "startUp" : "none",
                    "installDate" : null
                },
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
                    "version" : "2.0.0.0",
                    "startUp" : "none",
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
                    "startUp" : "none",
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
                    "startUp" : "none",
                    "installDate" : "2016-04-21T05:00:00+00:00"
                },
                ...
            }
        }
    }
}
</pre>

## Start/Stop Applications on The Device

To start or stop an install application on the device, the immediate method `"microsoft.management.manageAppLifeCycle"` can be called with the following parameters:

<pre>
{
    "action" : "start|stop",
    "pkgFamilyName" : "<i>pkgFamilyName</i>"
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
