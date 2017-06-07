# Windows Update Management

The **Windows Update Management** allows control over the following aspects:

- "**How**"
  - How the updates are applied (time, source, etc). This is grouped under **Windows Update Policy** and **Windows Update Reboot Policy**.
  - The policy is split into two because the update reboot policy is more likely to be set independently of of the rest of the update policies.
- "**What**"
  - What updates are installed, installable, pending, etc. This is group under **Windows Updates**.

## Windows Update Policy

### Configuration

The **Windows Update Policy** can be configured through the ```"windowsUpdatePolicy"``` node in the desired properties section as follows:

<pre>
"desired" : {
    "microsoft" : {
        "management" : {
            "windowsUpdatePolicy": <i>see below</i>
            }
        }
    }
}
</pre>

- ```"windowsUpdatePolicy"``` can be set to one of the following values:
  - ```"no-apply-no-report"``` : This means no desired state, and not reported state should be stored in the device twin.
  - ```"no-apply-yes-report"```: This means no desired state should be stored in the device twin, but reported state should.
  - <i>Device twin control properties</i> json object as follows:

<pre>
    {
        "applyProperties" : {
            "activeHoursStart": <i>see below</i>,
            "activeHoursEnd": <i>see below</i>,
            "allowAutoUpdate": <i>see below</i>,
            "allowUpdateService": <i>see below</i>,
            "branchReadinessLevel":  <i>see below</i>,
            "deferFeatureUpdatesPeriod": <i>see below</i>,
            "deferQualityUpdatesPeriod": <i>see below</i>,
            "pauseFeatureUpdates": <i>see below</i>,
            "pauseQualityUpdates": <i>see below</i>,
            "scheduledInstallDay": <i>see below</i>,
            "scheduledInstallTime": <i>see below</i>,
            "ring": <i>see below</i>
        }
        "reportProperties" : <i>see below</i>
    }
</pre>

- For a full documentation on what each field does, see the [Policy CSP](https://msdn.microsoft.com/en-us/windows/hardware/commercialize/customize/mdm/policy-configuration-service-provider) MSDN page.
- The ```"ring"``` field can be set to one of the following values:
  - ```"EarlyAdopter"```
  - ```"Preview"```
  - ```"GeneralAvailability"```
- The ```"reportProperties"``` can be set to one of the following values:
  - ```"yes"```: tells the DM client to report the Windows Update Policy state of the device.
  - ```"no"```: tells the DM client to not report the Windows Update Policy section in the reported properties. This can be useful to free some room in the Device Twin.

### Reporting

The device current state of the **Windows Update Policy** can be inspected through the ```"windowsUpdatePolicy"``` node in the reported properties section as follows:

<pre>
"reported" : {
    "microsoft" : {
        "management" : {
            "windowsUpdatePolicy": <i>see below</i>
        }
    }
}
</pre>


- ```"windowsUpdatePolicy"``` can be set to one of the following values:
  - ```"no-report"```
  - <i>Windows update policy properties</i> json object as follows:
<pre>
 {
    "activeHoursStart": <i>see below</i>,
    "activeHoursEnd": <i>see below</i>,
    "allowAutoUpdate": <i>see below</i>,
    "allowUpdateService": <i>see below</i>,
    "branchReadinessLevel":  <i>see below</i>,
    "deferFeatureUpdatesPeriod": <i>see below</i>,
    "deferQualityUpdatesPeriod": <i>see below</i>,
    "pauseFeatureUpdates": <i>see below</i>,
    "pauseQualityUpdates": <i>see below</i>,
    "scheduledInstallDay": <i>see below</i>,
    "scheduledInstallTime": <i>see below</i>,
    "ring": <i>see below</i>,
}
</pre>

- See documentation of individual properties under the Configuration section above.

## Windows Updates

### Configuration

The **Windows Updates** can be configured through the ```"windowsUpdates"``` node in the desired properties section as follows:

<pre>
"desired" : {
    "microsoft" : {
        "management" : {
            "windowsUpdates": {
                "approved": "<i>see below</i>"
            }
        }
    }
}
</pre>

For a full documentation on what each field does, see the [Update CSP](https://msdn.microsoft.com/en-us/windows/hardware/commercialize/customize/mdm/update-csp) MSDN page.

### Reporting

The device current state of the **Windows Updates** can be inspected through the ```"windowsUpdates"``` node in the reported properties section as follows:

<pre>
"reported" : {
    "microsoft" : {
        "management" : {
            "windowsUpdates": {
                "deferUpgrade": <i>see below</i>,
                "lastScanTime": "<i>see below</i>",
                "pendingReboot": "<i>see below</i>",
                "installable": "<i>see below</i>",
                "failed": "<i>see below</i>",
                "approved": "<i>see below</i>",
                "installed": "<i>see below</i>"
            }
        }
    }
}
</pre>

For a full documentation on what each field does, see the [Update CSP](https://msdn.microsoft.com/en-us/windows/hardware/commercialize/customize/mdm/update-csp) MSDN page.
