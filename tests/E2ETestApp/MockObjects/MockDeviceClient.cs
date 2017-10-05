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

using System;
using System.Collections.Generic;
using System.Threading.Tasks;

namespace Mock.Microsoft.Azure.Devices.Client
{
    public enum TransportType
    {
        Amqp = 0,
        Http1 = 1,
        Amqp_WebSocket_Only = 2,
        Amqp_Tcp_Only = 3,
        Mqtt = 4,
        Mqtt_WebSocket_Only = 5,
        Mqtt_Tcp_Only = 6
    }

    public enum ConnectionStatus
    {
        Disconnected = 1,
        Connected = 2,
        Disconnected_Retrying = 4,
        Disabled = 8
    }

    public enum ConnectionStatusChangeReason
    {
        Connection_Ok = 0,
        Expired_SAS_Token = 1,
        Device_Disabled = 2,
        Bad_Credential = 3,
        Retry_Expired = 4,
        No_Network = 5,
        Communication_Error = 6,
        Client_Close = 7
    }

    public sealed class MethodResponse
    {
        public MethodResponse(int status)
        {
            Status = status;
        }

        public MethodResponse(byte[] result, int status)
        {
            Result = result;
            Status = status;
        }

        public int Status { get; set; }
        public byte [] Result { get; set; }
    }

    public sealed class MethodRequest
    {
        public MethodRequest(string name, string dataAsJson)
        {
            Name = name;
            DataAsJson = dataAsJson;
        }

        public string Name { get; }
        public string DataAsJson { get; }
    }

    public delegate void ConnectionStatusChangesHandler(ConnectionStatus status, ConnectionStatusChangeReason reason);
    public delegate Task<MethodResponse> MethodCallback(MethodRequest methodRequest, object userContext);
    public delegate Task DesiredPropertyUpdateCallback(TwinCollection desiredProperties, object userContext);

    public class IotHubException : Exception
    {
        public IotHubException(string message) { }
        public IotHubException(Exception innerException) { }
        public IotHubException(string message, string trackingId) { }
        public IotHubException(string message, bool isTransient) { }
        public IotHubException(string message, Exception innerException) { }
        public IotHubException(string message, bool isTransient, string trackingId) { }
        protected IotHubException(string message, Exception innerException, bool isTransient) { }
        protected IotHubException(string message, Exception innerException, bool isTransient, string trackingId) { }

        public bool IsTransient { get; }
        public string TrackingId { get; set; }
    }

    public sealed class IotHubCommunicationException : IotHubException
    {
        public IotHubCommunicationException(string message) : base(message)
        {
        }

        public IotHubCommunicationException(string message, Exception innerException) : base(message, innerException)
        {
        }
    }

    public sealed class Message
    {
        public Message(byte[] byteArray)
        {

        }

        public IDictionary<string, string> Properties { get; }
    }

    public class DeviceClient
    {
        private const string JsonProperties = "properties";
        private const string JsonDesired = "desired";
        private const string JsonVersion = "$version";

        private DeviceClient(Twin twin)
        {
            _twin = twin;
            _twin.SetDesiredPropertyUpdateCallbackAsync(OnDesiredPropertyUpdated);
        }

        public static DeviceClient CreateFromConnectionString(Twin twin, string connectionString, TransportType transportType)
        {
            return new DeviceClient(twin);
        }

        public void SetConnectionStatusChangesHandler(ConnectionStatusChangesHandler statusChangesHandler)
        {
            _connectionStatusChangesHandler += statusChangesHandler;
        }

        public Task CloseAsync()
        {
            return Task.CompletedTask;
        }

        public Task<Twin> GetTwinAsync()
        {
            return Task.FromResult<Twin>(_twin);
        }

        public async Task UpdateReportedPropertiesAsync(TwinCollection reportedProperties)
        {
            await _twin.UpdateReported(reportedProperties);
        }

        public async Task SetMethodHandlerAsync(string methodName, MethodCallback methodHandler, object userContext)
        {
            await _twin.SetMethodHandlerAsync(methodName, methodHandler, userContext);
        }

        public Task SetDesiredPropertyUpdateCallbackAsync(DesiredPropertyUpdateCallback callback, object userContext)
        {
            _desiredPropertyUpdateCallback = callback;
            _desiredPropertyUpdateCallbackContext = userContext;
            return Task.CompletedTask;
        }

        public async Task OnDesiredPropertyUpdated(TwinCollection twinProperties)
        {
            if (_desiredPropertyUpdateCallback != null)
            {
                await _desiredPropertyUpdateCallback(twinProperties, _desiredPropertyUpdateCallbackContext);
            }
        }

        public Task SendEventAsync(Message message)
        {
            return Task.CompletedTask;
        }

        private Twin _twin;
        private ConnectionStatusChangesHandler _connectionStatusChangesHandler;
        private DesiredPropertyUpdateCallback _desiredPropertyUpdateCallback;
        private object _desiredPropertyUpdateCallbackContext;
    }
}