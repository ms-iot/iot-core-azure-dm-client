# Debugging Azure Device Management Client

All the **Windows IoT Azure DM Client** components have been instrumented to log its progress using ETW. The provider guid is `D198EE7D-C5F1-4F5F-95BE-A2EE6FA45897`.

The user can turn on event collection using either:

- [Diagnostic Logs Management](diagnostic-logs-management.md)
- xperf/tracerpt 

## xperf/tracerpt

Here are the steps you need to obtain detailed logs for a certain period of time:

- Open an admin console to the device and start capturing logs using:

    <pre>
    xperf.exe -start MySession -f c:\Data\Users\DefaultAccount\AppData\Local\Temp\dm.etl -on D198EE7D-C5F1-4F5F-95BE-A2EE6FA45897
    </pre>

- Run your scenarios (i.e. set the desired property in question, etc).
- When done, stop capturing logs using:

    <pre>
    xperf.exe -stop MySession
    </pre>

- To view the logs, copy the generated `c:\temp\dm.etl` to your desktop machine.
- View the etl log using Windows Performance Analyzer (WPA). 
  - If it is not installed, use `tracerpt.exe` to general an xml representation of the etl file. The xml file can then be opened as usual.

      <pre>
      tracerpt.exe c:\temp\dm.etl
      </pre>

##

----

[Home Page](../README.md) | [Library Reference](library-reference.md)