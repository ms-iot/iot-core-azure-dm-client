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
#r "Microsoft.ServiceBus"
#r "Microsoft.WindowsAzure.Storage"
#r "System.Xml"
#r "DMDataContract.dll"
#load "NonceTable.csx"
#load "NonceRequestHandler.csx"
#load "ReportRequestHandler.csx"
#load "HealthInspector.csx"

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
            await handler.ProcessAsync(dhaMsg);
        }
        else if (string.Compare(messageType, DeviceHealthAttestationDataContract.HealthReportTag) == 0)
        {
            var handler = new ReportRequestHandler(nonceTable, dhaReportTable, serviceClient, registryManager, log);
            var report = await handler.ProcessAsync(dhaMsg);

            var healthInspector = new HealthInspector(serviceClient, log);
            await healthInspector.ProcessHealthReportAsync(report);
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
