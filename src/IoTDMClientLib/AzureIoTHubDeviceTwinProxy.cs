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

using Newtonsoft.Json.Linq;
using Microsoft.Azure.Devices.Client;
using Microsoft.Azure.Devices.Client.Exceptions;
using Microsoft.Azure.Devices.Shared;
using Microsoft.Devices.Management.DMDataContract;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;
using System.Threading.Tasks;
using Windows.Foundation.Diagnostics;
using Windows.Networking.Connectivity;

namespace Microsoft.Devices.Management
{
    // This IDeviceTwin represents the actual Azure IoT Device Twin
    public class AzureIoTHubDeviceTwinProxy : IDeviceTwin
    {
        DeviceClient deviceClient;

        public delegate void LogAsync(string message, LoggingLevel level);
        LogAsync logAsyncHandler;

        public delegate Task ResetConnectionAsync(DeviceClient existingClient);
        ResetConnectionAsync resetConnectionAsyncHandler;

        public AzureIoTHubDeviceTwinProxy(DeviceClient deviceClient, ResetConnectionAsync resetConnectionAsyncHandler, LogAsync logAsyncHandler = null)
        {
            this.deviceClient = deviceClient;
            this.resetConnectionAsyncHandler = resetConnectionAsyncHandler;
            this.logAsyncHandler = logAsyncHandler;

            this.deviceClient.SetConnectionStatusChangesHandler(async (ConnectionStatus status, ConnectionStatusChangeReason reason) =>
            {
                string msg = "Connection changed: " + status.ToString() + " " + reason.ToString();
                System.Diagnostics.Debug.WriteLine(msg);
                logAsyncHandler?.Invoke(msg, LoggingLevel.Verbose);

                switch (reason)
                {
                    case ConnectionStatusChangeReason.Connection_Ok:
                        // No need to do anything, this is the expectation
                        break;

                    case ConnectionStatusChangeReason.Expired_SAS_Token:
                    case ConnectionStatusChangeReason.Bad_Credential:
                    case ConnectionStatusChangeReason.Retry_Expired:
                    case ConnectionStatusChangeReason.No_Network:
                        await InternalRefreshConnectionAsync();
                        break;

                    case ConnectionStatusChangeReason.Client_Close:
                        // ignore this ... part of client shutting down.
                        break;

                    case ConnectionStatusChangeReason.Communication_Error:
                    case ConnectionStatusChangeReason.Device_Disabled:
                        // These are not implemented in the Azure SDK
                        break;

                    default:
                        break;
                }
            });
        }

        public static Dictionary<string, object> DictionaryFromTwinCollection(TwinCollection twinProperties)
        {
            Dictionary<string, object> desiredProperties = new Dictionary<string, object>();
            foreach (KeyValuePair<string, object> p in twinProperties)
            {
                desiredProperties[p.Key] = p.Value;
            }
            desiredProperties["$version"] = twinProperties.Version;
            return desiredProperties;
        }

        async Task<Dictionary<string, object>> IDeviceTwin.GetDesiredPropertiesAsync()
        {
            Logger.Log("AzureIoTHubDeviceTwinProxy.GetDesiredPropertiesAsync", LoggingLevel.Information);

            var desiredProperties = new Dictionary<string, object>();
            try
            {
                var twin = await this.deviceClient.GetTwinAsync();
                foreach (KeyValuePair<string, object> p in twin.Properties.Desired)
                {
                    desiredProperties[p.Key] = p.Value;
                }
                desiredProperties[DMJSonConstants.DTVersionString] = twin.Properties.Desired.Version;
            }
            catch (IotHubCommunicationException e)
            {
                logAsyncHandler?.Invoke(e.ToString(), LoggingLevel.Error);
                await InternalRefreshConnectionAsync();
            }
            catch (Exception e)
            {
                logAsyncHandler?.Invoke(e.ToString(), LoggingLevel.Error);
            }

            return desiredProperties;
        }

        async Task<string> IDeviceTwin.GetAllPropertiesAsync()
        {
            Logger.Log("AzureIoTHubDeviceTwinProxy.GetAllPropertiesAsync", LoggingLevel.Information);
            StringBuilder sb = new StringBuilder();
            try
            {
                var twin = await this.deviceClient.GetTwinAsync();
                sb.Append("{\n");
                sb.Append("    \"properties\" : {\n");
                sb.Append("         \"desired\" : \n");
                sb.Append(twin.Properties.Desired.ToJson());
                sb.Append(",\n");
                sb.Append("         \"reported\" : \n");
                sb.Append(twin.Properties.Reported.ToJson());
                sb.Append("\n");
                sb.Append("    }\n");
                sb.Append("}\n");
            }
            catch (IotHubCommunicationException e)
            {
                logAsyncHandler?.Invoke(e.ToString(), LoggingLevel.Error);
                await InternalRefreshConnectionAsync();
            }
            catch (Exception e)
            {
                logAsyncHandler?.Invoke(e.ToString(), LoggingLevel.Error);
            }

            Debug.WriteLine("doc = " + sb.ToString());

            return sb.ToString();
        }

        async Task IDeviceTwin.ReportProperties(Dictionary<string, object> collection)
        {
            Logger.Log("AzureIoTHubDeviceTwinProxy.ReportProperties", LoggingLevel.Information);

            TwinCollection azureCollection = new TwinCollection();
            foreach (KeyValuePair<string, object> p in collection)
            {
                Logger.Log("  Reporting: " + p.Key, LoggingLevel.Information);
                if (p.Value is JObject)
                {
                    JObject jObject = (JObject)p.Value;
                    foreach (JProperty property in jObject.Children())
                    {
                        Logger.Log("    Reporting: " + property.Name, LoggingLevel.Information);
                    }
                }
                azureCollection[p.Key] = p.Value;
            }

            try
            {
                await this.deviceClient.UpdateReportedPropertiesAsync(azureCollection);
            }
            catch (IotHubCommunicationException e)
            {
                logAsyncHandler?.Invoke(e.ToString(), LoggingLevel.Error);
                await InternalRefreshConnectionAsync();
            }
            catch (Exception e)
            {
                logAsyncHandler?.Invoke(e.ToString(), LoggingLevel.Error);
            }
        }

        async Task IDeviceTwin.SetMethodHandlerAsync(string methodName, Func<string, Task<string>> methodHandler)
        {
            Logger.Log("AzureIoTHubDeviceTwinProxy.SetMethodHandlerAsync", LoggingLevel.Information);

            try
            {
                await this.deviceClient.SetMethodHandlerAsync(methodName, async (MethodRequest methodRequest, object userContext) =>
                {
                    var response = await methodHandler(methodRequest.DataAsJson);
                    return new MethodResponse(Encoding.UTF8.GetBytes(response), 0);
                }, null);
            }
            catch (IotHubCommunicationException e)
            {
                logAsyncHandler?.Invoke(e.ToString(), LoggingLevel.Error);
                await InternalRefreshConnectionAsync();
            }
            catch (Exception e)
            {
                logAsyncHandler?.Invoke(e.ToString(), LoggingLevel.Error);
            }
        }

        private static async Task WaitForInternet()
        {
            while (true)
            {
                ConnectionProfile connections = NetworkInformation.GetInternetConnectionProfile();
                bool internet = connections != null && connections.GetNetworkConnectivityLevel() != NetworkConnectivityLevel.None;
                if (internet) break;

                await Task.Delay(5 * 1000);
            }
        }

        async Task IDeviceTwin.RefreshConnectionAsync()
        {
            await InternalRefreshConnectionAsync();
        }

        private async Task InternalRefreshConnectionAsync()
        {
            while (true)
            {
                try
                {
                    await WaitForInternet();

                    var devicTwinImpl = this;
                    await devicTwinImpl.resetConnectionAsyncHandler(devicTwinImpl.deviceClient);
                    break;
                }
                catch (IotHubCommunicationException e)
                {
                    logAsyncHandler?.Invoke(e.ToString(), LoggingLevel.Error);
                }
                catch (Exception e)
                {
                    logAsyncHandler?.Invoke(e.ToString(), LoggingLevel.Error);
                }
                await Task.Delay(5 * 60 * 1000);
            }
        }

        async Task IDeviceTwin.SendMessageAsync(string messageContent, IDictionary<string, string> properties)
        {
            Logger.Log("AzureIoTHubDeviceTwinProxy.SendMessageAsync", LoggingLevel.Information);

            var message = new Microsoft.Azure.Devices.Client.Message(Encoding.UTF8.GetBytes(messageContent));
            foreach(var pair in properties)
            {
                message.Properties.Add(pair);
            }

            try
            {
                await this.deviceClient.SendEventAsync(message);
            }
            catch (IotHubCommunicationException e)
            {
                logAsyncHandler?.Invoke(e.ToString(), LoggingLevel.Error);
                await InternalRefreshConnectionAsync();
            }
            catch (Exception e)
            {
                logAsyncHandler?.Invoke(e.ToString(), LoggingLevel.Error);
            }
        }

        void IDeviceTwin.SignalOperationComplete()
        {
            // No such concept in Azure.
        }
    }
}
