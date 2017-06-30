/*
Copyright 2017 Microsoft
Permission is hereby granted, free of charge, to any person obtaining a copy of this software 
and associated documentation files (the "Software"), to deal in the Software without restriction, 
including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, 
subject to the following conditions:

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT 
LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH 
THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#r "Newtonsoft.Json"
#load "ReportRequestHandler.csx"

using System;
using System.Text;
using System.Xml;
using System.Threading.Tasks;
using Microsoft.Azure.Devices;
using Microsoft.Devices.Management.DMDataContract;
using Newtonsoft.Json;

class HealthInspector
{
    public HealthInspector(ServiceClient iotHubServiceClient, TraceWriter log)
    {
        _log = log;
        _iotHubServiceClient = iotHubServiceClient;
    }

    public async Task ProcessHealthReportAsync(HealthReport report)
    {
        if (!IsDeviceCompliance(report))
        {
            await FactoryResetAsync(report.DeviceId);
        }
    }

    private bool IsDeviceCompliance(HealthReport report)
    {
        return true;

        //
        // Sample code for inspecting the health report.
        //
        /*
        bool secureBootEnabled;
        if (!bool.TryParse(report.HealthCertificateProperties["SecureBootEnabled"], out secureBootEnabled))
        {
            return false;
        }

        return secureBootEnabled;
        */
    }

    private async Task FactoryResetAsync(string deviceId)
    {
        // Invoke device method
        dynamic param = new {
            recoveryPartitionGUID = "<your recovery partition GUID>",
            clearTPM = false
        };
        var json = JsonConvert.SerializeObject(param);
        var cloudToDeviceMethod = new CloudToDeviceMethod("microsoft.management.factoryReset");
        cloudToDeviceMethod.SetPayloadJson(json);

        var result = await _iotHubServiceClient.InvokeDeviceMethodAsync(deviceId, cloudToDeviceMethod);
        _log.Info($"FactoryResetAsync status: {result.Status}");
    }

    private ServiceClient _iotHubServiceClient;
    private TraceWriter _log;
}
