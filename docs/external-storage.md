# Blob Storage

Some device management configurations require the user to provide input files (like installing new certificates, or WiFi profiles, etc). Also, some scenarios may result in having the device generate files that need to be later transferred ourside the device (like log files, exporting WiFi profiles, certificates, etc).

For these scenarios, the device twin specifies a common storage place for all configurations to use. The current implementaiton of the device management client on IoT Core assumes this is an Azure Blob Storage service.

The storage is described in the desired properties section as follows:
<pre>
  "properties": {
    "desired" : {
      "microsoft": {
        "management": {
          "externalStorage": {
            "connectionString": "<i>connection string</i>",
          }
        }
      }
    }
  }
</pre>

The inidividual configuration section may then refer to files in that location - either at the root or under containers. The relative location is left for the individual sections to define.


