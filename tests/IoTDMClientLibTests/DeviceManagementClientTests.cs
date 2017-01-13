using Microsoft.Devices.Management;
using Microsoft.VisualStudio.TestPlatform.UnitTestFramework;
using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using Windows.Storage.Streams;

namespace IoTDMClientLibTests
{
    class TwinMockup : IDeviceTwin
    {
        void IDeviceTwin.SetManagementClient(DeviceManagementClient deviceManagementClient)
        {
            throw new NotImplementedException();
        }

        void IDeviceTwin.RefreshConnection()
        {
            throw new NotImplementedException();
        }

        void IDeviceTwin.ReportProperties(Dictionary<string, object> collection)
        {
            throw new NotImplementedException();
        }
    }

    class HandlerMockup : IDeviceManagementRequestHandler
    {
        string applicationName;
        SystemRebootRequestResponse rebootResponse;

        public HandlerMockup(string applicationName, SystemRebootRequestResponse rebootResponse)
        {
            this.applicationName = applicationName;
            this.rebootResponse = rebootResponse;
        }

        Task<ApplicationInfo> IDeviceManagementRequestHandler.GetApplicationInfo()
        {
            var appinfo = new ApplicationInfo();
            appinfo.ApplicationName = applicationName;
            return Task.FromResult<ApplicationInfo>(appinfo);
        }

        Task<SystemRebootRequestResponse> IDeviceManagementRequestHandler.IsSystemRebootAllowed()
        {
            return Task.FromResult<SystemRebootRequestResponse>(rebootResponse);
        }
    }

    class ConfigurationProxyMockup : ISystemConfiguratorProxy
    {
        DMMessage receivedMessage = new DMMessage { Context = (uint)DMCommand.Unknown };
        public Task<DMMessage> SendCommandAsync(DMMessage command)
        {
            this.receivedMessage = command;
            var response = new DMMessage { Context = 0 };
            return Task.FromResult<DMMessage>(response);
        }

        public DMMessage ReceivedMessage => this.receivedMessage;
    }

    [TestClass]
    public class DeviceManagementClientTests
    {
        [TestMethod]
        public void MockupProxyImmediateRebootTest()
        {
            var twin = new TwinMockup();
            var requestHandler = new HandlerMockup("Test", SystemRebootRequestResponse.StartNow);
            var proxy = new ConfigurationProxyMockup();
            var dmClient = DeviceManagementClient.Create(twin, requestHandler, proxy);
            dmClient.RebootSystemAsync().Wait();

            Assert.AreEqual<uint>(proxy.ReceivedMessage.Context, (uint)DMCommand.RebootSystem);
        }

        [TestMethod]
        public void MockupProxyPostponedRebootTest()
        {
            var twin = new TwinMockup();
            var requestHandler = new HandlerMockup("Test", SystemRebootRequestResponse.AskAgainLater);
            var proxy = new ConfigurationProxyMockup();
            var dmClient = DeviceManagementClient.Create(twin, requestHandler, proxy);
            dmClient.RebootSystemAsync().Wait();

            Assert.AreEqual<uint>(proxy.ReceivedMessage.Context, (uint)DMCommand.Unknown);
        }
    }
}