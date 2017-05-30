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
using System.Threading.Tasks;
using Microsoft.VisualStudio.TestPlatform.UnitTestFramework;
using Microsoft.Devices.Management;
using Windows.Storage.Streams;
using System.Runtime.InteropServices.WindowsRuntime;

using Microsoft.Devices.Management.Message;
using System.Collections.Generic;
using Newtonsoft.Json.Linq;
using Microsoft.Devices.Management.DMDataContract;
using System.Threading;
using System.Reflection;
using Newtonsoft.Json;

namespace IoTDMClientLibTests
{
    [TestClass]
    public class DeviceHealthAttestationTest
    {
        class CallBackMockup : IClientHandlerCallBack
        {
            public Task SendMessageAsync(string message, IDictionary<string, string> properties)
            {
                if (SendMessageHook != null)
                {
                    SendMessageHook(message, properties);
                    return Task.CompletedTask;
                }
                throw new NotImplementedException();
            }

            public Task ReportPropertiesAsync(string propertyName, JObject properties)
            {
                if (ReportPropertiesHook != null)
                {
                    ReportPropertiesHook(propertyName, properties);
                    return Task.CompletedTask;
                }
                throw new NotImplementedException();
            }

            public Action<string, IDictionary<string, string>> SendMessageHook;
            public Action<string, JObject> ReportPropertiesHook;
        }


        class ConfigurationProxyMockup : ISystemConfiguratorProxy
        {
            public Task<IResponse> SendCommandAsync(IRequest request)
            {
                if (SendCommandHook != null)
                {
                    return Task.FromResult<IResponse>(SendCommandHook(request));
                }
                throw new NotImplementedException();
            }

            public Func<IRequest, IResponse> SendCommandHook;
        }

        public DeviceHealthAttestationTest()
        {
            reportedPropertEvent = new AutoResetEvent(/*initialState=*/false);
            sentMessageEvent = new AutoResetEvent(/*initalState=*/false);
        }

        [TestMethod]
        public void TestDesiredPropertySingleReport()
        {
            // Expected values
            var expectedEndpoint = "Dummy Endpoint";

            // Mock Object setup
            var callback = new CallBackMockup();
            callback.SendMessageHook = SendMessageHook;
            callback.ReportPropertiesHook = ReportPropertiesHook;

            var systemConfigurator = new ConfigurationProxyMockup();
            systemConfigurator.SendCommandHook = (IRequest request) =>
            {
                if (request.Tag == DMMessageKind.DeviceHealthAttestationVerifyHealth)
                {
                    var request2 = (DeviceHealthAttestationVerifyHealthRequest)request;
                    Assert.AreEqual(expectedEndpoint, request2.HealthAttestationServerEndpoint);
                    return new StatusCodeResponse(ResponseStatus.Success, DMMessageKind.DeviceHealthAttestationVerifyHealth);
                }
                Assert.Fail($"Got unexpected command - {request.Tag}");
                return null;
            };

            var dha = new DeviceHealthAttestationHandler(callback, systemConfigurator);

            // Test begins
            var desiredProperty = new DeviceHealthAttestationDataContract.DesiredProperty();
            desiredProperty.Endpoint = expectedEndpoint;
            desiredProperty.ReportIntervalInSeconds = 0;
            dha.OnDesiredPropertyChange(JObject.FromObject(desiredProperty));

            // Test validation
            Assert.IsTrue(sentMessageEvent.WaitOne(TimeSpan.FromSeconds(5)));
            Assert.AreEqual(DeviceHealthAttestationDataContract.NonceRequestTag, sentMessageProperties["MessageType"]);

            EnsureReportedPropertyStatus("Nonce requested");
        }

        [TestMethod]
        public void TestDesiredPropertyRecurringReporting()
        {
            // Expected values
            var expectedEndpoint = "Dummy Endpoint";

            // Mock Object setup
            var callback = new CallBackMockup();
            callback.SendMessageHook = SendMessageHook;
            callback.ReportPropertiesHook = ReportPropertiesHook;


            var systemConfigurator = new ConfigurationProxyMockup();
            systemConfigurator.SendCommandHook = (IRequest request) =>
            {
                if (request.Tag == DMMessageKind.DeviceHealthAttestationVerifyHealth)
                {
                    var request2 = (DeviceHealthAttestationVerifyHealthRequest)request;
                    Assert.AreEqual(expectedEndpoint, request2.HealthAttestationServerEndpoint);
                    return new StatusCodeResponse(ResponseStatus.Success, DMMessageKind.DeviceHealthAttestationVerifyHealth);
                }
                Assert.Fail($"Got unexpected command - {request.Tag}");
                return null;
            };

            var dha = new DeviceHealthAttestationHandler(callback, systemConfigurator);

            // Test begins
            try
            {
                var desiredProperty = new DeviceHealthAttestationDataContract.DesiredProperty();
                desiredProperty.Endpoint = expectedEndpoint;
                desiredProperty.ReportIntervalInSeconds = 1;
                dha.OnDesiredPropertyChange(JObject.FromObject(desiredProperty));

                // Test validation
                for (int i = 0; i < 3; i++)
                {
                    Assert.IsTrue(sentMessageEvent.WaitOne(TimeSpan.FromSeconds(5)));
                    Assert.AreEqual(DeviceHealthAttestationDataContract.NonceRequestTag, sentMessageProperties["MessageType"]);
                }
            }
            finally
            {
                // Disable the reporting
                var desiredProperty = new DeviceHealthAttestationDataContract.DesiredProperty();
                desiredProperty.Endpoint = "";
                desiredProperty.ReportIntervalInSeconds = -1;
                dha.OnDesiredPropertyChange(JObject.FromObject(desiredProperty));
            }
        }

        [TestMethod]
        public void TestDesiredPropertyDisabledReporting()
        {
            // Mock Object setup
            var callback = new CallBackMockup();
            callback.ReportPropertiesHook = ReportPropertiesHook;
            var systemConfigurator = new ConfigurationProxyMockup();
            var dha = new DeviceHealthAttestationHandler(callback, systemConfigurator);

            // Test begins
            var desiredProperty = new DeviceHealthAttestationDataContract.DesiredProperty();
            desiredProperty.Endpoint = "";
            desiredProperty.ReportIntervalInSeconds = -1;
            dha.OnDesiredPropertyChange(JObject.FromObject(desiredProperty));

            // Test validation
            EnsureReportedPropertyStatus("Reporting Disabled");
        }

        [TestMethod]
        public void TestDesiredPropertyInvalidJObject()
        {
            // Send invalid JObject as desired property.  Ensure error is reflected in the reported property.
            var callback = new CallBackMockup();
            callback.ReportPropertiesHook = ReportPropertiesHook;
            var systemConfigurator = new ConfigurationProxyMockup();
            var dha = new DeviceHealthAttestationHandler(callback, systemConfigurator);

            {
                // Test 1: Endpoint = null
                dha.OnDesiredPropertyChange(new JObject());
                EnsureReportedPropertyStatus("VerifyHealth failed: Value cannot be null.");
            }

            {
                // Test 2: 
                var jobj = new JObject();
                jobj.Add("ReportIntervalInSeconds", "string in an integer field");
                dha.OnDesiredPropertyChange(jobj);
                EnsureReportedPropertyStatus("VerifyHealth failed: Could not convert string to integer: string in an integer field. Path 'ReportIntervalInSeconds'.");
            }
        }

        [TestMethod]
        public void TestDirectMethodGetCertificate()
        {
            // Expected values
            var expectedNonce = "AFAFAFAFAF";
            var expectedHealthCert = "Dummy health cert";
            var expectedCorrelationId = "Dummy correlation id";

            // Mock Object setup
            var callback = new CallBackMockup();
            callback.SendMessageHook = SendMessageHook;
            callback.ReportPropertiesHook = ReportPropertiesHook;

            var systemConfigurator = new ConfigurationProxyMockup();
            systemConfigurator.SendCommandHook = (IRequest request) =>
            {
                if (request.Tag == DMMessageKind.DeviceHealthAttestationGetReport)
                {
                    var request2 = (DeviceHealthAttestationGetReportRequest)request;
                    Assert.AreEqual(expectedNonce, request2.Nonce);
                    return new DeviceHealthAttestationGetReportResponse(expectedHealthCert, expectedCorrelationId);
                }
                Assert.Fail($"Got unexpected command - {request.Tag}");
                return null;
            };

            var dha = new DeviceHealthAttestationHandler(callback, systemConfigurator);

            // Test begins
            var getCertificateMethod = dha.GetDirectMethodHandler()[DeviceHealthAttestationDataContract.GetReportMethodName];
            var param = new DeviceHealthAttestationDataContract.GetReportMethodParam() { Nonce = expectedNonce };

            var response = getCertificateMethod(JsonConvert.SerializeObject(param)).Result;

            // Test validation
            Assert.IsTrue(sentMessageEvent.WaitOne(TimeSpan.FromSeconds(5)));
            Assert.AreEqual(SuccessResponse, response);
            Assert.AreEqual(DeviceHealthAttestationDataContract.HealthReportTag, sentMessageProperties["MessageType"]);
            var message = JsonConvert.DeserializeObject<DeviceHealthAttestationDataContract.HealthReport>(sentMessaage);
            Assert.AreEqual(expectedCorrelationId, message.CorrelationId);
            Assert.AreEqual(expectedHealthCert, message.HealthCertificate);
            EnsureReportedPropertyStatus("Reported");
        }

        private void EnsureReportedPropertyStatus(string expectedStatus)
        {
            Assert.IsTrue(reportedPropertEvent.WaitOne(TimeSpan.FromSeconds(5)));
            Assert.AreEqual(DeviceHealthAttestationDataContract.JsonSectionName, actualReportedPropertyName);
            var actualReportedProperty = actualReportedPropertyJObject.ToObject<DeviceHealthAttestationDataContract.ReportedProperty>();
            Assert.AreEqual(expectedStatus, actualReportedProperty.status);
        }

        private void ReportPropertiesHook(string propertyName, JObject properties)
        {
            actualReportedPropertyName = propertyName;
            actualReportedPropertyJObject = properties;
            reportedPropertEvent.Set();
        }

        private void SendMessageHook(string message, IDictionary<string, string> properties)
        {
            sentMessageEvent.Set();
            sentMessaage = message;
            sentMessageProperties = properties;
        }

        private readonly string SuccessResponse = "{\"response\":\"succeeded\",\"reason\":\"\"}";

        private AutoResetEvent reportedPropertEvent;
        private string actualReportedPropertyName;
        private JObject actualReportedPropertyJObject;

        private AutoResetEvent sentMessageEvent;
        private string sentMessaage;
        private IDictionary<string, string> sentMessageProperties;
    }
}
