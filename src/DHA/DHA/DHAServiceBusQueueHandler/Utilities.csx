#r "Newtonsoft.Json"
#r "Microsoft.ServiceBus"
#r "System.Xml"


using System;
using System.IO;
using System.Net;
using System.Text;
using System.Xml;
using System.Threading.Tasks;
using Microsoft.Azure.Devices;
using Microsoft.ServiceBus.Messaging;
using Microsoft.WindowsAzure.Storage.Table;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;


public static string GetPartitionKeyForDeviceId(string deviceId)
{
    return "";
}

public static string GetBodyString(this BrokeredMessage msg)
{
    using (var stream = msg.GetBody<Stream>())
    {
        if (stream == null)
        {
            return null;
        }

        using (StreamReader reader = new StreamReader(stream))
        {
            return reader.ReadToEnd();
        }
    }
}


