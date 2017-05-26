#r "Newtonsoft.Json"
#r "Microsoft.ServiceBus"
#r "Microsoft.WindowsAzure.Storage"
#r "System.Xml"
#r "DMDataContract.dll"
#load "NonceTable.csx"
#load "Utilities.csx"

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

class ReportRequestHandler
{
    static readonly string DefaultEndpoint = "https://has.spserv.microsoft.com";
    static readonly string ValidateHealthCertificatePath = "/DeviceHealthAttestation/ValidateHealthCertificate/v3";

    public ReportRequestHandler(CloudTable nonceTable, CloudTable reportTable, ServiceClient iotHubServiceClient, RegistryManager registryManager, TraceWriter log)
    {
        _log = log;
        _nonceTable = new NonceCloudTable(nonceTable, log);
        _reportTable = reportTable;
        _iotHubServiceClient = iotHubServiceClient;
        _registryManager = registryManager;
    }
    
    public async Task Process(BrokeredMessage dhaMsg)
    {
        var deviceId = dhaMsg.Properties["iothub-connection-device-id"].ToString();
        _log.Info($"deviceId: {deviceId}");

        var msg = dhaMsg.GetBodyString();
        if (msg == null) throw new ArgumentException("HealthReport message body cannot be null");
        _log.Info($"Message received: {msg}");

        var request = JsonConvert.DeserializeObject<DeviceHealthAttestationDataContract.HealthReport>(msg);
        if (string.IsNullOrWhiteSpace(request.CorrelationId)) throw new ArgumentException("CorrelationId is empty");
        if (string.IsNullOrWhiteSpace(request.HealthCertificate)) throw new ArgumentException("HealthCertificate is empty");
        _log.Info($"request.CorrelationId: {request.CorrelationId}");
        _log.Info($"request.HealthCertificate: {request.HealthCertificate}");

        // Decode the health certificate
        var bytes = Convert.FromBase64String(request.HealthCertificate);
        var xmlStream = new MemoryStream(bytes);
        var xml = new XmlDocument();
        xml.Load(xmlStream);

        // Add the nonce
        var nonce = await _nonceTable.GetNonceForDeviceAsync(deviceId);
        await _nonceTable.DeleteNonceForDeviceAsync(deviceId);
        var xmlRoot = xml.DocumentElement;
        ValidateNodeName(xmlRoot, "HealthCertificateValidationRequest");
        var nonceElement = xml.CreateElement("Nonce", xmlRoot.NamespaceURI);
        nonceElement.InnerText = nonce;
        xmlRoot.PrependChild(nonceElement);
        _log.Info(xml.InnerXml);

        // Get HAS Endpoint
        var url = await GetHasServerForDeviceAsync(deviceId);
        _log.Info($"HasServerUrl: {url}");

        // Validate HealthCertificate with HAS server
        var report = await PostHttpRequestToServer(url, xml.InnerXml);
        _log.Info($"report: {report}");

        // Translate DHA-report to Azure table entity
        var entity = ParseHealthCertificateValidationResponse(report);
        entity.PartitionKey = GetPartitionKeyForDeviceId(deviceId);
        entity.RowKey = string.Format("{0}-{1}", deviceId, DateTime.UtcNow.ToString("o"));
        entity.Properties.Add("DeviceId", new EntityProperty(deviceId));
        entity.Properties.Add("CorrelationId", new EntityProperty(request.CorrelationId));
        entity.Properties.Add("HealthCertificate", new EntityProperty(request.HealthCertificate));

        // Insert report into the table
        var insertOperation = TableOperation.Insert(entity);
        await _reportTable.ExecuteAsync(insertOperation);
    }

    private async Task<string> GetHasServerForDeviceAsync(string deviceId)
    {
        var twin = await _registryManager.GetTwinAsync(deviceId);
        if (twin == null)
        {
            var msg = string.Format($"Device twin does not exist.  DeviceId: {deviceId}");
            throw new InvalidOperationException(msg);
        }

        dynamic json = twin.Properties.Desired.ToJson();
        var obj = JsonConvert.DeserializeObject(json);
        var endpoint = obj.microsoft.management.deviceHealthAttestation.Endpoint.ToString();
        
        if (endpoint == null || string.Compare(endpoint, "") == 0 || string.Compare(endpoint, "DEFAULT", /*ignorecase=*/true) == 0)
        {
            endpoint = DefaultEndpoint;
        }
        return endpoint.TrimEnd('/') + ValidateHealthCertificatePath;
    }

    private void ValidateNodeName(XmlNode node, string expectedName)
    {
        if (string.Compare(node.Name, expectedName) != 0)
        {
            var msg = $"Invalid XML node - Expected: {expectedName} Got: {node.Name}";
            throw new InvalidOperationException(msg);
        }
    }

    private DynamicTableEntity ParseHealthCertificateValidationResponse(string report)
    {
        var xml = new XmlDocument();
        xml.LoadXml(report);

        var root = xml.DocumentElement;
        ValidateNodeName(root, "HealthCertificateValidationResponse");

        var healthCertificatePropertiesNode = root.FirstChild;
        ValidateNodeName(healthCertificatePropertiesNode, "HealthCertificateProperties");

        var entity = new DynamicTableEntity();
        foreach (XmlNode node in healthCertificatePropertiesNode.ChildNodes)
        {
            var name = $"HealthCertificateProperties_{node.Name}";
            var value = node.InnerText;
            entity.Properties.Add(name, new EntityProperty(value));
        }
        return entity;
    }

    private static async Task<string> PostHttpRequestToServer(string url, string xml)
    {
        byte[] data = Encoding.UTF8.GetBytes(xml);
        WebRequest request = WebRequest.Create(url);
        request.Method = "POST";
        request.ContentType = "application/octet-stream";
        request.ContentLength = data.Length;
        using (Stream stream = request.GetRequestStream())
        {
            stream.Write(data, 0, data.Length);
        }

        // Get the response
        using (var response = await request.GetResponseAsync())
        {
            using (var stream = response.GetResponseStream())
            {
                using (var streamReader = new StreamReader(stream))
                {
                    return streamReader.ReadToEnd();
                }
            }
        }
    }

    private NonceCloudTable _nonceTable;
    private CloudTable _reportTable;
    private ServiceClient _iotHubServiceClient;
    private RegistryManager _registryManager;
    private TraceWriter _log;
}
