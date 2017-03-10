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

This policy can be used in cases where the application is busy and need to disable system update reboots until it is no longer busy. The application developer can invoke that functionality using
the following .Net APIs.

<pre>
    <b>Namespace</b>: Microsoft.Devices.Management
</pre>

<pre>
    <b>Class</b>: DeviceManagementClient
</pre>

<pre>
    <b>Methods</b>:
    public async Task&lt;ResponseStatus&gt; SetWindowsUpdateRebootPolicyAsync(bool allowReboots)
    public async Task&lt;bool&gt; GetWindowsUpdateRebootPolicy()
</pre>

**Example**

<pre>
    async Task OnStartCriticalTask(DeviceManagementClient dmClient)
    {
        ResponseStatus status = await dmClient.SetWindowsUpdateRebootPolicyAsync(false);
        if (status == ResponseStatus.Failure)
        {
            throw new Exception("Failed to disable update reboots!");
        }
        return StartCriticalTask();
    }

    void OnCriticalTaskDone(DeviceManagementClient dmClient)
    {
        dmClient.SetWindowsUpdateRebootPolicyAsync(true);
    }
</pre>

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
