#Windows Update Management

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
            "windowsUpdatePolicy": {
                "activeHoursStart": <i>see below</i>,
                "activeHoursEnd": <i>see below</i>,
                "allowAutoUpdate": <i>see below</i>,
                "allowMUUpdateService": <i>see below</i>,
                "allowNonMicrosoftSignedUpdate": <i>see below</i>,
                "allowUpdateService": <i>see below</i>,
                "branchReadinessLevel":  <i>see below</i>,
                "deferFeatureUpdatesPeriod": <i>see below</i>,
                "deferQualityUpdatesPeriod": <i>see below</i>,
                "excludeWUDrivers": <i>see below</i>,
                "pauseFeatureUpdates": <i>see below</i>,
                "pauseQualityUpdates": <i>see below</i>,
                "requireUpdateApproval": <i>see below</i>,
                "scheduledInstallDay": <i>see below</i>,
                "scheduledInstallTime": <i>see below</i>,
                "updateServiceUrl": "<i>see below</i>",
            }
        }
    }
}
</pre>

For a full documentation on what each field does, see the [Policy CSP](https://msdn.microsoft.com/en-us/windows/hardware/commercialize/customize/mdm/policy-configuration-service-provider) MSDN page.

### Reporting

The device current state of the **Windows Update Policy** can be inspected through the ```"windowsUpdatePolicy"``` node in the reported properties section as follows:

<pre>
"reported" : {
    "microsoft" : {
        "management" : {
            "windowsUpdatePolicy": {
                "activeHoursStart": <i>see below</i>,
                "activeHoursEnd": <i>see below</i>,
                "allowAutoUpdate": <i>see below</i>,
                "allowMUUpdateService": <i>see below</i>,
                "allowNonMicrosoftSignedUpdate": <i>see below</i>,
                "allowUpdateService": <i>see below</i>,
                "branchReadinessLevel":  <i>see below</i>,
                "deferFeatureUpdatesPeriod": <i>see below</i>,
                "deferQualityUpdatesPeriod": <i>see below</i>,
                "excludeWUDrivers": <i>see below</i>,
                "pauseFeatureUpdates": <i>see below</i>,
                "pauseQualityUpdates": <i>see below</i>,
                "requireUpdateApproval": <i>see below</i>,
                "scheduledInstallDay": <i>see below</i>,
                "scheduledInstallTime": <i>see below</i>,
                "updateServiceUrl": "<i>see below</i>",
            }
        }
    }
}
</pre>

For a full documentation on what each field does, see the [Policy CSP](https://msdn.microsoft.com/en-us/windows/hardware/commercialize/customize/mdm/policy-configuration-service-provider) MSDN page.

## Windows Update Reboot Policy

This policy can be used in cases where the operator or the application need to inform the system that the device should not reboot to complete a Windows Update installation. This can be useful if the device is in the middle of performing a critical task, for example.

### Configuration

The **Windows Update Reboot Policy** can be configured through the ```"windowsUpdateRebootPolicy"``` node in the desired properties section as follows:

<pre>
"desired" : {
    "microsoft" : {
        "management" : {
          "windowsUpdateRebootPolicy": {
              "allow": false|true
            },
        }
    }
}
</pre>

- ```"allow"```:
  - When this is set to ```"true"```, the Windows Update subsystem will proceed with rebooting to complete the installation of newly downloaded updates.
  - When this is set to ```"false"```, the Windows Update subsystem will not proceed with rebooting until this is set to true or until the maximum amount of update deferral is reached.

### Reporting

The device current state of the **Windows Update Reboot Policy** can be inspected through the ```"windowsUpdateRebootPolicy"``` node in the reported properties section as follows:

<pre>
"reported" : {
    "microsoft" : {
        "management" : {
          "windowsUpdateRebootPolicy": {
              "allow": false|true
            },
        }
    }
}
</pre>

- ```"allow"```:
  - The meaning is the same as defined under the **configuration** section.

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
"desired" : {
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
