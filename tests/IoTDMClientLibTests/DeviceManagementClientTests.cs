using Microsoft.Devices.Management;
using Microsoft.VisualStudio.TestPlatform.UnitTestFramework;
using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using Windows.Storage.Streams;

using Microsoft.Devices.Management.Message;

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

    class HandlerMockupForReboot : IDeviceManagementRequestHandler
    {
        SystemRebootRequestResponse rebootResponse;

        public HandlerMockupForReboot(SystemRebootRequestResponse rebootResponse)
        {
            this.rebootResponse = rebootResponse;
        }

        Task<ApplicationInfo> IDeviceManagementRequestHandler.GetApplicationInfo()
        {
            throw new NotImplementedException();
        }

        Task<SystemRebootRequestResponse> IDeviceManagementRequestHandler.IsSystemRebootAllowed()
        {
            return Task.FromResult<SystemRebootRequestResponse>(rebootResponse);
        }
    }

    class HandlerMockupForAppInstall : IDeviceManagementRequestHandler
    {
        public string AppName;
        public HandlerMockupForAppInstall(string appName){ AppName = appName; }

        Task<ApplicationInfo> IDeviceManagementRequestHandler.GetApplicationInfo()
        {
            throw new NotImplementedException();
        }

        Task<SystemRebootRequestResponse> IDeviceManagementRequestHandler.IsSystemRebootAllowed()
        {
            throw new NotImplementedException();
        }
    }

    class ConfigurationProxyMockup : ISystemConfiguratorProxy
    {
        IRequest request = null;
        IResponse response = null;

        public Task<IResponse> SendCommandAsync(IRequest request)
        {
            this.request = request;
            // Sending bytes over to SystemConfigurator...
            if (request.Tag == DMMessageKind.InstallApp)
            {
                var appinstallReq = (AppInstallRequest)request; // cast must succeed
                this.response = new AppInstallResponse(ResponseStatus.Success);
                return Task.FromResult<IResponse>(response);
            }
            else if (request.Tag == DMMessageKind.RebootSystem)
            {
                var appinstallReq = (RebootRequest)request; // cast must succeed
                this.response = new StatusCodeResponse(ResponseStatus.Success, DMMessageKind.RebootSystem);
                return Task.FromResult<IResponse>(response);
            }
            else throw new Exception("Unsupported command");
        }

        public IRequest ReceivedRequest => this.request;
        public IResponse ReturnedResponse => this.response;
    }

    [TestClass]
    public class DeviceManagementClientTests
    {
        [TestMethod]
        public void MockupProxyImmediateRebootTest()
        {
            var twin = new TwinMockup();
            var requestHandler = new HandlerMockupForReboot(SystemRebootRequestResponse.StartNow);
            var proxy = new ConfigurationProxyMockup();
            var dmClient = DeviceManagementClient.Create(twin, requestHandler, proxy);
            dmClient.RebootSystemAsync().Wait();

            Assert.AreEqual(proxy.ReceivedRequest.Tag, DMMessageKind.RebootSystem);
            Assert.AreEqual(proxy.ReturnedResponse.Tag, DMMessageKind.RebootSystem);
            Assert.AreEqual(proxy.ReturnedResponse.Status, ResponseStatus.Success);
        }

        [TestMethod]
        public void MockupProxyPostponedRebootTest()
        {
            var twin = new TwinMockup();
            var requestHandler = new HandlerMockupForReboot(SystemRebootRequestResponse.AskAgainLater);
            var proxy = new ConfigurationProxyMockup();
            var dmClient = DeviceManagementClient.Create(twin, requestHandler, proxy);
            dmClient.RebootSystemAsync().Wait();

            Assert.AreEqual(proxy.ReceivedRequest, null);
            Assert.AreEqual(proxy.ReturnedResponse, null);
        }

        [TestMethod]
        public void MockupProxyInstallAppTest()
        {
            var twin = new TwinMockup();
            var appname = "abc";
            var proxy = new ConfigurationProxyMockup();

            var response = proxy.SendCommandAsync(new AppInstallRequest(appname)).Result;

            Assert.AreEqual(response.Status, ResponseStatus.Success);
            Assert.AreEqual(response.Tag, DMMessageKind.InstallApp);
        }
    }
}