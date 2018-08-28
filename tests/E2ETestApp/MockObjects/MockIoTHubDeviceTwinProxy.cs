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
using Microsoft.Devices.Management;
using Microsoft.Devices.Management.DMDataContract;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;
using System.Threading.Tasks;
using Windows.Foundation.Diagnostics;

using Mock.Microsoft.Azure.Devices.Client;

namespace Mock.Lib
{
    // This is an identical implementation to the AzureIoTHubDeviceTwinProxy provided in the Toaster sample.
    // The difference is that it is using mock objects for Azure.

    public class AzureIoTHubDeviceTwinProxy : IDeviceTwin
    {
        public delegate void LogAsync(string message, LoggingLevel level);
        public delegate Task ResetConnectionAsync(DeviceClient existingClient, Twin twin);

        public AzureIoTHubDeviceTwinProxy(DeviceClient deviceClient, Twin twin, ResetConnectionAsync resetConnectionAsyncHandler, LogAsync logAsyncHandler = null)
        {
            _deviceClient = deviceClient;
            _twin = twin;
            _resetConnectionAsyncHandler = resetConnectionAsyncHandler;
            _logAsyncHandler = logAsyncHandler;

            _deviceClient.SetConnectionStatusChangesHandler(async (ConnectionStatus status, ConnectionStatusChangeReason reason) =>
            {
                string msg = "Mock.Lib.Connection changed: " + status.ToString() + " " + reason.ToString();
                Debug.WriteLine(msg);
                _logAsyncHandler?.Invoke(msg, LoggingLevel.Verbose);

                switch (reason)
                {
                    case ConnectionStatusChangeReason.Connection_Ok:
                        // No need to do anything, this is the expectation
                        break;

                    case ConnectionStatusChangeReason.Retry_Expired:
                        await InternalRefreshConnectionAsync();
                        break;

                    case ConnectionStatusChangeReason.Client_Close:
                        // ignore this ... part of client shutting down.
                        break;

                    case ConnectionStatusChangeReason.Communication_Error:
                    case ConnectionStatusChangeReason.Expired_SAS_Token:
                    case ConnectionStatusChangeReason.Bad_Credential:
                    case ConnectionStatusChangeReason.Device_Disabled:
                        // These are not implemented in the Azure SDK
                        break;

                    case ConnectionStatusChangeReason.No_Network:
                    // This seems to lead to Retry_Expired, so we can 
                    // ignore this ... maybe log the error.

                    default:
                        break;
                }
            });
        }

        async Task<Dictionary<string, object>> IDeviceTwin.GetDesiredPropertiesAsync()
        {
            Logger.Log("Mock.Lib.AzureIoTHubDeviceTwinProxy.GetDesiredPropertiesAsync", LoggingLevel.Information);

            var desiredProperties = new Dictionary<string, object>();
            try
            {
                var twin = await _deviceClient.GetTwinAsync();
                if (twin.Properties != null && twin.Properties.Desired != null)
                {
                    foreach (KeyValuePair<string, object> p in twin.Properties.Desired)
                    {
                        desiredProperties[p.Key] = p.Value;
                    }
                    desiredProperties[DMJSonConstants.DTVersionString] = twin.Properties.Desired.Version;
                }
            }
            catch (IotHubCommunicationException e)
            {
                _logAsyncHandler?.Invoke(e.ToString(), LoggingLevel.Error);
                await InternalRefreshConnectionAsync();
            }
            catch (Exception e)
            {
                _logAsyncHandler?.Invoke(e.ToString(), LoggingLevel.Error);
            }

            return desiredProperties;
        }

        async Task<string> IDeviceTwin.GetAllPropertiesAsync()
        {
            Logger.Log("Mock.Lib.AzureIoTHubDeviceTwinProxy.GetAllPropertiesAsync", LoggingLevel.Information);
            StringBuilder sb = new StringBuilder();
            try
            {
                var twin = await _deviceClient.GetTwinAsync();
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
                _logAsyncHandler?.Invoke(e.ToString(), LoggingLevel.Error);
                await InternalRefreshConnectionAsync();
            }
            catch (Exception e)
            {
                _logAsyncHandler?.Invoke(e.ToString(), LoggingLevel.Error);
            }

            Debug.WriteLine("doc = " + sb.ToString());

            return sb.ToString();
        }

        async Task IDeviceTwin.ReportProperties(Dictionary<string, object> collection)
        {
            Logger.Log("Mock.Lib.AzureIoTHubDeviceTwinProxy.ReportProperties", LoggingLevel.Information);

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
                await _deviceClient.UpdateReportedPropertiesAsync(azureCollection);
            }
            catch (IotHubCommunicationException e)
            {
                _logAsyncHandler?.Invoke(e.ToString(), LoggingLevel.Error);
                await InternalRefreshConnectionAsync();
            }
            catch (Exception e)
            {
                _logAsyncHandler?.Invoke(e.ToString(), LoggingLevel.Error);
            }
        }

        async Task IDeviceTwin.SetMethodHandlerAsync(string methodName, Func<string, Task<string>> methodHandler)
        {
            Logger.Log("Mock.Lib.AzureIoTHubDeviceTwinProxy.SetMethodHandlerAsync", LoggingLevel.Information);

            try
            {
                await _deviceClient.SetMethodHandlerAsync(methodName, async (MethodRequest methodRequest, object userContext) =>
                {
                    var response = await methodHandler(methodRequest.DataAsJson);
                    return new MethodResponse(Encoding.UTF8.GetBytes(response), 0);
                }, null);
            }
            catch (IotHubCommunicationException e)
            {
                _logAsyncHandler?.Invoke(e.ToString(), LoggingLevel.Error);
                await InternalRefreshConnectionAsync();
            }
            catch (Exception e)
            {
                _logAsyncHandler?.Invoke(e.ToString(), LoggingLevel.Error);
            }
        }

        private async Task InternalRefreshConnectionAsync()
        {
            while (true)
            {
                try
                {
                    await _resetConnectionAsyncHandler(_deviceClient, _twin);
                    break;
                }
                catch (IotHubCommunicationException e)
                {
                    _logAsyncHandler?.Invoke(e.ToString(), LoggingLevel.Error);
                }
                catch (Exception e)
                {
                    _logAsyncHandler?.Invoke(e.ToString(), LoggingLevel.Error);
                }
                await Task.Delay(5 * 60 * 1000);
            }
        }

        async Task IDeviceTwin.SendMessageAsync(string messageContent, IDictionary<string, string> properties)
        {
            Logger.Log("Mock.Lib.AzureIoTHubDeviceTwinProxy.SendMessageAsync", LoggingLevel.Information);

            var message = new Message(Encoding.UTF8.GetBytes(messageContent));
            foreach (var pair in properties)
            {
                message.Properties.Add(pair);
            }

            try
            {
                await _deviceClient.SendEventAsync(message);
            }
            catch (IotHubCommunicationException e)
            {
                _logAsyncHandler?.Invoke(e.ToString(), LoggingLevel.Error);
                await InternalRefreshConnectionAsync();
            }
            catch (Exception e)
            {
                _logAsyncHandler?.Invoke(e.ToString(), LoggingLevel.Error);
            }
        }

        public void SignalOperationComplete()
        {
            Logger.Log("Mock.Lib.AzureIoTHubDeviceTwinProxy.SendMessageAsync", LoggingLevel.Information);

            _twin.SignalOperationComplete();
        }

        DeviceClient _deviceClient;
        Twin _twin;
        LogAsync _logAsyncHandler;
        ResetConnectionAsync _resetConnectionAsyncHandler;
    }
}
