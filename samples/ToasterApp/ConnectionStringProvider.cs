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
        public static string Value => "HostName=GMilekaStd1.azure-devices.net;DeviceId=gmileka02;SharedAccessKey=+iU29auWae7W+L/g8UjC1Bn/kToBI5OKpFtLsuucOB0=";
    }
}