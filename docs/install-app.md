## Install App

The **Install App** operation is initiated by the device receiving the `microsoft.management.appInstall` method.

## Input Payload 
Input payload contains Json according to this format:

<pre>
{
  "PackageFamilyName": <the App PackageFamilyName>,
  "Appx": {
    "ConnectionString": <connection string to Azure blob>,
    "ContainerName": <container name for Azure blob>,
    "BlobName": <Azure blob name for App>
  },
  "Dependencies": [
    {
      "ConnectionString": <connection string to Azure blob>,
      "ContainerName": <container name for Azure blob>,
      "BlobName": <Azure blob name for App dependency>
    }
  ]
}
</pre>

## Output Payload
The device responds immediately with the following JSON payload:

<pre>
"response" : value (<i>See below</i>)
</pre>


Possible `"response"` values are: 
- `"install succeeded"` - The app (and associated dependencies) successfully installed.
- `"install failed"` - The app did not install successfully.

