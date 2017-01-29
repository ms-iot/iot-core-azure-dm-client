using Microsoft.Azure.Devices.Client;
using Microsoft.Azure.Devices.Shared;
using Microsoft.Devices.Management;
using System;
using System.Diagnostics;
using System.Threading.Tasks;
using Windows.UI.Core;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;

namespace Toaster
{
    public static class ConnectionStringProvider
    {
        public static string Value => "HostName=iot-open-house-demo.azure-devices.net;DeviceId=dog-feeder;SharedAccessKey=Rt3JTLyI3Rj4ZCNm6pPkT8TKHTw1TMLMNnXd8u/pp9g=";
    }
}