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

using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Threading;
using System.Threading.Tasks;

namespace Mock.Microsoft.Azure.Devices.Client
{
    public class TwinProperties
    {
        public TwinProperties() { }

        public TwinCollection Desired { get; set; }
        public TwinCollection Reported { get; set; }
    }

    public delegate Task DesiredPropertyUpdateCallbackInternal(TwinCollection desiredProperties);
    public delegate Task ReportedPropertyUpdateCallbackInternal(TwinCollection reportedProperties);

    public class Twin
    {
        private const string JsonProperties = "properties";
        private const string JsonDesired = "desired";
        private const string JsonReported = "reported";
        private const string JsonVersion = "$version";

        class MethodHandler
        {
            public MethodCallback _callback;
            public object _userContext;

            public MethodHandler(MethodCallback callback, object userContext)
            {
                _callback = callback;
                _userContext = userContext;
            }
        }

        public string DeviceId { get; set; }
        public TwinProperties Properties { get; set; }

        public Twin(string deviceId, string json)
        {
            JObject twinData = (JObject)JsonConvert.DeserializeObject(json);

            _directMethods = new Dictionary<string, MethodHandler>();
            _deviceId = deviceId;
            _twinData = twinData;
            _desiredVersion = 1;
            _reportedVersion = 1;

            JToken propertiesToken;
            if (!_twinData.TryGetValue(JsonProperties, out propertiesToken) || propertiesToken == null)
            {
                throw new Exception("Twin: Missing required property: " + JsonProperties);
            }

            if (!(propertiesToken is JObject))
            {
                throw new Exception("Twin: Invalid property type: " + JsonProperties);
            }

            JObject propertiesObject = (JObject)propertiesToken;

            JToken desiredToken;
            if (!propertiesObject.TryGetValue(JsonDesired, out desiredToken) || desiredToken == null)
            {
                throw new Exception("Twin: Missing required property: " + JsonDesired);
            }

            if (!(desiredToken is JObject))
            {
                throw new Exception("Twin: Invalid property type: " + JsonDesired);
            }

            JToken reportedToken;
            if (!propertiesObject.TryGetValue(JsonReported, out reportedToken) || reportedToken == null)
            {
                throw new Exception("Twin: Missing required property: " + JsonReported);
            }

            if (!(reportedToken is JObject))
            {
                throw new Exception("Twin: Invalid property type: " + JsonReported);
            }

            JObject desiredObject = (JObject)desiredToken;
            JObject reportedObject = (JObject)reportedToken;


            Properties = new TwinProperties();
            Properties.Desired = new TwinCollection(desiredObject);
            Properties.Reported = new TwinCollection(reportedObject);
        }

        public async Task UpdateDesired(string jsonString)
        {
            // Update version...
            ++_desiredVersion;
            _twinData[JsonProperties][JsonDesired][JsonVersion] = _desiredVersion;

            // Merge...
            JObject jsonObject = (JObject)JsonConvert.DeserializeObject(jsonString);
            _twinData.Merge(jsonObject);

            // Notify...
            if (_desiredPropertyUpdateCallback != null)
            {
                await _desiredPropertyUpdateCallback(Properties.Desired);
            }
        }

        public async Task<MethodResponse> InvokeDirectMethod(string methodName, string parameters)
        {
            MethodResponse methodResponse = new MethodResponse(0);

            _manualResetEvent = new ManualResetEvent(false);

            MethodHandler methodHandler;
            if (_directMethods.TryGetValue(methodName, out methodHandler))
            {
                MethodRequest request = new MethodRequest(methodName, parameters);
                methodResponse = await methodHandler._callback(request, methodHandler._userContext);
            }

            // Wait for SignalOperationComplete() to be called before returning.
            // This is useful for testing async mehtods.

            await Task.Run(() => { _manualResetEvent.WaitOne(5000); });
            _manualResetEvent = null;
            return methodResponse;
        }

        public void SignalOperationComplete()
        {
            if (_manualResetEvent != null)
            {
                _manualResetEvent.Set();
            }
        }

        public Task SetMethodHandlerAsync(string methodName, MethodCallback methodHandler, object userContext)
        {
            _directMethods[methodName] = new MethodHandler(methodHandler, userContext);
            return Task.CompletedTask;
        }

        public async Task UpdateReported(TwinCollection reportedProperties)
        {
            // Update version...
            ++_reportedVersion;
            _twinData[JsonProperties][JsonReported][JsonVersion] = _reportedVersion;

            // Convert and construct a merge-able JObject...
            JObject reportedObject = new JObject();
            foreach (KeyValuePair<string, object> pair in reportedProperties)
            {
                reportedObject[pair.Key] = (JToken)pair.Value;
            }

            JObject propertiesObject = new JObject();
            propertiesObject[JsonReported] = reportedObject;

            JObject rootObject = new JObject();
            rootObject[JsonProperties] = propertiesObject;

            // Merge...
            _twinData.Merge(rootObject);


            // Notify...
            if (_reportedPropertyUpdateCallback != null)
            {
                await _reportedPropertyUpdateCallback(reportedProperties);
            }
        }

        public void SetDesiredPropertyUpdateCallbackAsync(DesiredPropertyUpdateCallbackInternal callback)
        {
            _desiredPropertyUpdateCallback = callback;
        }

        public void SetReportedPropertyUpdateCallbackAsync(ReportedPropertyUpdateCallbackInternal callback)
        {
            _reportedPropertyUpdateCallback = callback;
        }

        private DesiredPropertyUpdateCallbackInternal _desiredPropertyUpdateCallback;
        private ReportedPropertyUpdateCallbackInternal _reportedPropertyUpdateCallback;

        private string _deviceId;
        private JObject _twinData;
        private uint _desiredVersion;
        private uint _reportedVersion;
        private Dictionary<string, MethodHandler> _directMethods;
        private ManualResetEvent _manualResetEvent;
    }
}