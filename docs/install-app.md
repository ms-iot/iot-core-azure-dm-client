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
{
    "response" : value (<i>See below</i>)
    "reason" : value (<i>See below</i>)
}
</pre>


Possible `"response"` values are: 
- `"accepted"` - The parameters sent were parsed and the App Install request was accepted.
- `"rejected"` - The parameters sent failed to parsed and the App Install request was rejected.  See the reason field for explanation.

`"reason"` is used to communicate why an App Install request was rejected if possible.

