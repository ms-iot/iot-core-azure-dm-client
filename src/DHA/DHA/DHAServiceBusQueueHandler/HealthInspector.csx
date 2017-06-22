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
