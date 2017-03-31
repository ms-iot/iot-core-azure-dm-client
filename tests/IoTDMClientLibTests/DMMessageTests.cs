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

namespace IoTDMClientLibTests
{
    [TestClass]
    public class DMMessageTests
    {
        [TestMethod]
        public void TestRequestSerializeDeserialize()
        {
            var appInstallRequest = new AppInstallRequest(new AppInstallInfo() { AppxPath = "abc", PackageFamilyName = "def", Dependencies = new List<String>() { "ghi", "jkl" } });
            var blob = appInstallRequest.Serialize();
            var appInstallRequestRehydrated = AppInstallRequest.Deserialize(blob) as AppInstallRequest;

            Assert.AreEqual(appInstallRequestRehydrated.Tag, DMMessageKind.InstallApp);
            Assert.AreEqual(appInstallRequestRehydrated.AppInstallInfo.AppxPath, "abc");
            Assert.AreEqual(appInstallRequestRehydrated.AppInstallInfo.PackageFamilyName, "def");
            Assert.AreEqual(appInstallRequestRehydrated.AppInstallInfo.Dependencies[0], "ghi");
            Assert.AreEqual(appInstallRequestRehydrated.AppInstallInfo.Dependencies[1], "jkl");
        }

        [TestMethod]
        public void TestReadFromIInputStream()
        {
            var command = new AppInstallRequest(new AppInstallInfo() { AppxPath = "abc", PackageFamilyName = "def", Dependencies = new List<String>() { "ghi", "jkl" } });
            var dataArray = command.Serialize().GetByteArrayForSerialization();
            var dataSizeArray = BitConverter.GetBytes((UInt32)dataArray.Length);

            var stream = new InMemoryRandomAccessStream();
            stream.WriteAsync(dataSizeArray.AsBuffer()).AsTask().Wait();
            stream.WriteAsync(dataArray.AsBuffer()).AsTask().Wait();

            var result = Blob.ReadFromIInputStreamAsync(stream.GetInputStreamAt(0)).AsTask().Result;

            Assert.AreEqual(result.Tag, DMMessageKind.InstallApp);

            var request = result.MakeIRequest() as AppInstallRequest;

            Assert.AreEqual(request.Tag, DMMessageKind.InstallApp);
            Assert.AreEqual(request.AppInstallInfo.AppxPath, "abc");
            Assert.AreEqual(request.AppInstallInfo.PackageFamilyName, "def");
            Assert.AreEqual(request.AppInstallInfo.Dependencies[0], "ghi");
            Assert.AreEqual(request.AppInstallInfo.Dependencies[1], "jkl");
        }

        [TestMethod]
        public void TestWriteToOutputStream()
        {
            var command = new AppInstallRequest(new AppInstallInfo() { AppxPath = "abc", PackageFamilyName = "def", Dependencies = new List<String>() { "ghi", "jkl" } });
            var blob = command.Serialize();

            var stream = new InMemoryRandomAccessStream();
            blob.WriteToIOutputStreamAsync(stream.GetOutputStreamAt(0)).AsTask().Wait();

            var reader = new DataReader(stream.GetInputStreamAt(0));
            reader.LoadAsync(4).AsTask().Wait();
            var bytes = new byte[4];
            reader.ReadBytes(bytes);
            var size = BitConverter.ToUInt32(bytes, 0);
            // like I said, too brittle:
            //Assert.AreEqual(size, 44U); // this is somewhat brittle.

            var reader2 = new DataReader(stream.GetInputStreamAt(4));
            reader2.LoadAsync(size).AsTask().Wait();
            var bytes2 = new byte[size];
            reader2.ReadBytes(bytes2);

            var blob2 = Blob.CreateFromByteArray(bytes2);

            Assert.AreEqual(blob.Tag, blob2.Tag);

            var command2 = blob2.MakeIRequest() as AppInstallRequest;
            Assert.AreEqual(command2.Tag, DMMessageKind.InstallApp);
            Assert.AreEqual(command2.AppInstallInfo.AppxPath, "abc");
            Assert.AreEqual(command2.AppInstallInfo.PackageFamilyName, "def");
            Assert.AreEqual(command2.AppInstallInfo.Dependencies[0], "ghi");
            Assert.AreEqual(command2.AppInstallInfo.Dependencies[1], "jkl");
        }


        [TestMethod]
        public void TestSerializationRoundtripThroughStream()
        {
            var command = new AppInstallRequest(new AppInstallInfo() { AppxPath = "abc", PackageFamilyName = "def", Dependencies = new List<String>() { "ghi", "jkl" } });
            var blob = command.Serialize();

            var stream = new InMemoryRandomAccessStream();

            blob.WriteToIOutputStreamAsync(stream.GetOutputStreamAt(0)).AsTask().Wait();
            var blob2 = Blob.ReadFromIInputStreamAsync(stream.GetInputStreamAt(0)).AsTask().Result;

            Assert.AreEqual(blob2.Tag, blob.Tag);

            var command2 = blob2.MakeIRequest() as AppInstallRequest;

            Assert.IsNotNull(command2);
            Assert.AreEqual(command2.Tag, DMMessageKind.InstallApp);
            Assert.AreEqual(command2.AppInstallInfo.AppxPath, "abc");
            Assert.AreEqual(command2.AppInstallInfo.PackageFamilyName, "def");
            Assert.AreEqual(command2.AppInstallInfo.Dependencies[0], "ghi");
            Assert.AreEqual(command2.AppInstallInfo.Dependencies[1], "jkl");

        }

        [TestMethod]
        public void TestRequestSendToProxy()
        {
            var appInstallRequest = new AppInstallRequest(new AppInstallInfo() { AppxPath = "abc", PackageFamilyName = "def", Dependencies = new List<String>() { "ghi", "jkl" } });
            var proxy = new ConfigurationProxyMockup();

            IResponse response = proxy.SendCommandAsync(appInstallRequest).Result;
            Assert.AreEqual(response.Status, ResponseStatus.Success);
        }

    }
}
