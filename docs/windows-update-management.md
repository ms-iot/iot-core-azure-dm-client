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
            "windowsUpdatePolicy": &lt;<i>see below</i>&gt;
            }
        }
    }
}
</pre>

- ```"windowsUpdatePolicy"``` can be set to one of the following:
    - ```"no-apply-no-report"``` : This means no desired state, and no reported state will be stored in the device twin.
  - ```"no-apply-yes-report"```: This means no desired state will be stored in the device twin, but reported state will.
  - A json object of the following format:

<pre>
    {
        "applyProperties" : {
            "activeHoursStart": &lt;<i>see below</i>&gt;,
            "activeHoursEnd": &lt;<i>see below</i>&gt;,
            "allowAutoUpdate": &lt;<i>see below</i>&gt;,
            "allowUpdateService": &lt;<i>see below</i>&gt;,
            "branchReadinessLevel":  &lt;<i>see below</i>&gt;,
            "deferFeatureUpdatesPeriod": &lt;<i>see below</i>&gt;,
            "deferQualityUpdatesPeriod": &lt;<i>see below</i>&gt;,
            "pauseFeatureUpdates": &lt;<i>see below</i>&gt;,
            "pauseQualityUpdates": &lt;<i>see below</i>&gt;,
            "scheduledInstallDay": &lt;<i>see below</i>&gt;,
            "scheduledInstallTime": &lt;<i>see below</i>&gt;,
            "ring": &lt;<i>see below</i>&gt;
        }
        "reportProperties" : &lt;<i>see below</i>&gt;
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
            "windowsUpdatePolicy": &lt;<i>see below</i>&gt;
        }
    }
}
</pre>


- ```"windowsUpdatePolicy"``` can be set to one of the following values:
  - ```"no-report"```
  - A json object of the following format:
<pre>
 {
    "activeHoursStart": &lt;<i>see below</i>&gt;,
    "activeHoursEnd": &lt;<i>see below</i>&gt;,
    "allowAutoUpdate": &lt;<i>see below</i>&gt;,
    "allowUpdateService": &lt;<i>see below</i>&gt;,
    "branchReadinessLevel":  &lt;<i>see below</i>&gt;,
    "deferFeatureUpdatesPeriod": &lt;<i>see below</i>&gt;,
    "deferQualityUpdatesPeriod": &lt;<i>see below</i>&gt;,
    "pauseFeatureUpdates": &lt;<i>see below</i>&gt;,
    "pauseQualityUpdates": &lt;<i>see below</i>&gt;,
    "scheduledInstallDay": &lt;<i>see below</i>&gt;,
    "scheduledInstallTime": &lt;<i>see below</i>&gt;,
    "ring": &lt;<i>see below</i>&gt;,
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
                "approved": "&lt;<i>see below</i>&gt;"
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
                "deferUpgrade": &lt;<i>see below</i>&gt;,
                "lastScanTime": "&lt;<i>see below</i>&gt;",
                "pendingReboot": "&lt;<i>see below</i>&gt;",
                "installable": "&lt;<i>see below</i>&gt;",
                "failed": "&lt;<i>see below</i>&gt;",
                "approved": "&lt;<i>see below</i>&gt;",
                "installed": "&lt;<i>see below</i>&gt;"
            }
        }
    }
}
</pre>

For a full documentation on what each field does, see the [Update CSP](https://msdn.microsoft.com/en-us/windows/hardware/commercialize/customize/mdm/update-csp) MSDN page.
