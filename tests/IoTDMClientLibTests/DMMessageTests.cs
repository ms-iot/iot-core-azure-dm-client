using System;
using System.Threading.Tasks;
using Microsoft.VisualStudio.TestPlatform.UnitTestFramework;
using Microsoft.Devices.Management;
using Windows.Storage.Streams;
using System.Runtime.InteropServices.WindowsRuntime;

using Microsoft.Devices.Management.Message;
using System.Collections.Generic;

namespace IoTDMClientLibTests
{
    [TestClass]
    public class DMMessageTests
    {
        [TestMethod]
        public void TestRequestSerializeDeserialize()
        {
            var appxname = "MyApp.appx";

            IRequest appInstallRequest = new AppInstallRequest(appxname);

            Blob blob = appInstallRequest.Serialize();

            AppInstallRequest appInstallRequestRehydrated = AppInstallRequest.Deserialize(blob);

            Assert.AreEqual(appInstallRequestRehydrated.Tag, DMMessageKind.InstallApp);
            Assert.AreEqual(appInstallRequestRehydrated.AppName, appxname);
        }

        [TestMethod]
        public void TestReadFromIInputStream()
        {
            var appname = "test";
            var command = new AppInstallRequest(appname);
            var dataArray = command.Serialize().GetByteArrayForSerialization();
            var dataSizeArray = BitConverter.GetBytes((UInt32)dataArray.Length);

            var stream = new InMemoryRandomAccessStream();
            stream.WriteAsync(dataSizeArray.AsBuffer()).AsTask().Wait();
            stream.WriteAsync(dataArray.AsBuffer()).AsTask().Wait();

            var result = Blob.ReadFromIInputStreamAsync(stream.GetInputStreamAt(0)).AsTask().Result;

            Assert.AreEqual(result.Tag, DMMessageKind.InstallApp);

            var request = result.MakeIRequest() as AppInstallRequest;

            Assert.IsNotNull(request);
            Assert.AreEqual(request.AppName, appname);
        }

        [TestMethod]
        public void TestWriteToOutputStream()
        {
            var appname = "test";
            var command = new AppInstallRequest(appname);
            var blob = command.Serialize();

            var stream = new InMemoryRandomAccessStream();
            blob.WriteToIOutputStreamAsync(stream.GetOutputStreamAt(0)).AsTask().Wait();

            var reader = new DataReader(stream.GetInputStreamAt(0));
            reader.LoadAsync(4).AsTask().Wait();
            var bytes = new byte[4];
            reader.ReadBytes(bytes);
            var size = BitConverter.ToUInt32(bytes, 0);
            Assert.AreEqual(size, 44U); // this is somewhat brittle.

            var reader2 = new DataReader(stream.GetInputStreamAt(4));
            reader2.LoadAsync(size).AsTask().Wait();
            var bytes2 = new byte[size];
            reader2.ReadBytes(bytes2);

            var blob2 = Blob.CreateFromByteArray(bytes2);

            Assert.AreEqual(blob.Tag, blob2.Tag);

            var command2 = blob2.MakeIRequest() as AppInstallRequest;
            Assert.AreEqual(command2.AppName, appname);
        }


        [TestMethod]
        public void TestSerializationRoundtripThroughStream()
        {
            var appname = "xyz";
            var command = new AppInstallRequest(appname);
            var blob = command.Serialize();

            var stream = new InMemoryRandomAccessStream();

            blob.WriteToIOutputStreamAsync(stream.GetOutputStreamAt(0)).AsTask().Wait();
            var blob2 = Blob.ReadFromIInputStreamAsync(stream.GetInputStreamAt(0)).AsTask().Result;

            Assert.AreEqual(blob2.Tag, blob.Tag);

            var command2 = blob2.MakeIRequest() as AppInstallRequest;

            Assert.IsNotNull(command2);
            Assert.AreEqual(command2.AppName, appname);

        }

        [TestMethod]
        public void TestRequestSendToProxy()
        {
            var appInstallRequest = new AppInstallRequest("MyApp.appx");

            var proxy = new ConfigurationProxyMockup();

            IResponse response = proxy.SendCommandAsync(appInstallRequest).Result;

            var typedResult = (AppInstallResponse)response; // cast must succeed

            Assert.AreEqual(typedResult.Status, ResponseStatus.Success);
        }

    }
}
