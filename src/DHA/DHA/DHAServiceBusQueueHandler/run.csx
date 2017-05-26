#r "Newtonsoft.Json"
#r "Microsoft.ServiceBus"
#r "Microsoft.WindowsAzure.Storage"
#r "System.Xml"
#r "DMDataContract.dll"
#load "NonceTable.csx"
#load "NonceRequestHandler.csx"
#load "ReportRequestHandler.csx"

using System;
using System.IO;
using System.Net;
using System.Text;
using System.Xml;
using System.Threading.Tasks;
using Microsoft.Azure.Devices;
using Microsoft.Devices.Management.DMDataContract;
using Microsoft.ServiceBus.Messaging;
using Microsoft.WindowsAzure.Storage.Table;
using Newtonsoft.Json;

// Requires "service" permission
static readonly string IoTHubConnectionString = Environment.GetEnvironmentVariable("iothub-service");
static ServiceClient serviceClient = ServiceClient.CreateFromConnectionString(IoTHubConnectionString);
static RegistryManager registryManager = RegistryManager.CreateFromConnectionString(IoTHubConnectionString);

public static async Task Run(BrokeredMessage dhaMsg, CloudTable nonceTable, CloudTable dhaReportTable, TraceWriter log)
{
    try
    {
        var messageType = dhaMsg.Properties["MessageType"].ToString();
        log.Info($"messageType: {messageType}");

        var nonceCloudTable = new NonceCloudTable(nonceTable, log);
        if (string.Compare(messageType, DeviceHealthAttestationDataContract.NonceRequestTag) == 0)
        {
            var handler = new NonceRequestHandler(nonceTable, serviceClient, log);
            await handler.Process(dhaMsg);
        }
        else if (string.Compare(messageType, DeviceHealthAttestationDataContract.HealthReportTag) == 0)
        {
            var handler = new ReportRequestHandler(nonceTable, dhaReportTable, serviceClient, registryManager, log);
            await handler.Process(dhaMsg);
        }
        else
        {
            log.Error($"Unknown messageType: {messageType}");
        }
    }
    catch (Exception e)
    {
        log.Error(e.ToString());
    }
}