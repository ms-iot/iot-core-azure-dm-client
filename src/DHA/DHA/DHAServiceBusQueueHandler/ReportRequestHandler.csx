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

class HealthReport
{
    public HealthReport(string deviceId, 
                        string correlationId, 
                        string healthCertificate, 
                        Dictionary<string, string> healthCertificateProperties)
    {
        if (string.IsNullOrWhiteSpace(deviceId)) throw new ArgumentException("deviceId");
        if (string.IsNullOrWhiteSpace(correlationId)) throw new ArgumentException("correlationId");
        if (string.IsNullOrWhiteSpace(healthCertificate)) throw new ArgumentException("healthCertificate");
        if (healthCertificateProperties == null) throw new ArgumentNullException("healthCertificateProperties");

        this.DeviceId = deviceId;
        this.CorrelationId = correlationId;
        this.HealthCertificate = healthCertificate;
        this.HealthCertificateProperties = healthCertificateProperties;
    }

    public string DeviceId { get; private set; }
    public string CorrelationId { get; private set; }
    public string HealthCertificate { get; private set; }
    public IReadOnlyDictionary<string, string> HealthCertificateProperties { get; private set; }

}

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
    
    public async Task<HealthReport> ProcessAsync(BrokeredMessage dhaMsg)
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
        var reportString = await PostHttpRequestToServerAsync(url, xml.InnerXml);
        _log.Info($"report: {reportString}");

        // Translate xml into HealthReport
        var properties = ParseHealthCertificateValidationResponse(reportString);
        var report = new HealthReport(deviceId, request.CorrelationId, request.HealthCertificate, properties);

        await WriteReportAsync(report);
        return report;
    }

    private async Task WriteReportAsync(HealthReport report)
    {
        // Create table entity for insertion
        var entity = new DynamicTableEntity();
        entity.PartitionKey = GetPartitionKeyForDeviceId(report.DeviceId);
        entity.RowKey = string.Format("{0}-{1}", report.DeviceId, DateTime.UtcNow.ToString("o"));
        entity.Properties.Add("DeviceId", new EntityProperty(report.DeviceId));
        entity.Properties.Add("CorrelationId", new EntityProperty(report.CorrelationId));
        entity.Properties.Add("HealthCertificate", new EntityProperty(report.HealthCertificate));
        foreach (var pair in report.HealthCertificateProperties)
        {
            entity.Properties.Add($"HealthCertificateProperties_{pair.Key}", new EntityProperty(pair.Value));
        }

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

    private Dictionary<string, string> ParseHealthCertificateValidationResponse(string report)
    {
        var xml = new XmlDocument();
        xml.LoadXml(report);

        var root = xml.DocumentElement;
        ValidateNodeName(root, "HealthCertificateValidationResponse");

        var healthCertificatePropertiesNode = root.FirstChild;
        ValidateNodeName(healthCertificatePropertiesNode, "HealthCertificateProperties");

        var properties = new Dictionary<string, string>();
        foreach (XmlNode node in healthCertificatePropertiesNode.ChildNodes)
        {
            properties.Add(node.Name, node.InnerText);
        }
        return properties;
    }

    private static async Task<string> PostHttpRequestToServerAsync(string url, string xml)
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
