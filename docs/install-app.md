## Install App

The **Install App** operation is initiated by the device receiving the `microsoft.management.appInstall` method. The method starts the process of downloading and installing an Appx package along with its dependencies. The application is hosted in an Azure blob, access to which is provided to the client via the connection string, as specified below.

## Input Payload 
Input payload contains Json according to this format:

```
{
  "PackageFamilyName" : "MyCoolApp_1abcde2f3ghij",
  "Appx": {
    "ConnectionString" : "DefaultEndpointsProtocol=https;AccountName=myaccount;AccountKey=blah",
    "ContainerName" : "MyAppContainer",
    "BlobName" : "MyCoolApp.appx"
  },
  "Dependencies": [
    {
      "ConnectionString" : "DefaultEndpointsProtocol=https;AccountName=myaccount;AccountKey=blah",
      "ContainerName" : "MyDependencyContainer",
      "BlobName" : "MyCoolAppDependency.appx"
    }
  ]
}
```

## Output Payload
The device responds immediately with the following JSON payload:

```
{
    "response" : value (See below)
    "reason" : value (See below)
}
```


Possible `"response"` values are: 
- `"accepted"` - The parameters sent were parsed and the App Install request was accepted.
- `"rejected"` - The parameters sent failed to parsed and the App Install request was rejected.  See the reason field for explanation.

`"reason"` is used to communicate why an App Install request was rejected if possible.

