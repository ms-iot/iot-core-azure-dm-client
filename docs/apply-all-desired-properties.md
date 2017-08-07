## Apply All Desired Properties

The **Apply All Desired Propeties** operation is initiated by the following .NET APIs.


<pre>
    <b>Namespace</b>:
    Microsoft.Devices.Management
</pre>

<pre>
    <b>Class</b>:
    DeviceManagementClient
</pre>

<pre>
    <b>Methods</b>:
    public async Task ApplyDesiredStateAsync()
    public void ApplyDesiredStateAsync(TwinCollection desiredProperties)
</pre>

**public async Task ApplyDesiredStateAsync()**

This tells the DM Client to get the full list of desired properties and apply them to the device. This is specially useful on application start-up since there are no guarantees about the current state of the device. Later, however, the device twin will send change notification for only the properties that have changed. In those cases, the application should call `ApplyDesiredStateAsync(TwinCollection desiredProperties)` instead - which applys only the delta specified.

**public void ApplyDesiredStateAsync(TwinCollection desiredProperties)**

This tells the DM Client to apply the specified desired state to the device. This should be used whenever the device twin sends a notification of a desired property change.

**Example**

<pre>
    async Task OnApplicationStartup()
    {
        await ApplyDesiredStateAsync();
    }

    void OnDesiredPropertiesChanged(TwinCollection desiredProperties)
    {
        await ApplyDesiredStateAsync(desiredProperties);
    }
</pre>

----

[Home Page](../README.md) | [Library Reference](library-reference.md)