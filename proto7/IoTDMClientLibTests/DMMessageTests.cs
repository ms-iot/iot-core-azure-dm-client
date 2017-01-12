using System;
using System.Threading.Tasks;
using Microsoft.VisualStudio.TestPlatform.UnitTestFramework;
using Microsoft.Devices.Management;
using Windows.Storage.Streams;
using System.Runtime.InteropServices.WindowsRuntime;

namespace IoTDMClientLibTests
{
    [TestClass]
    public class DMMessageTests
    {
        [TestMethod]
        public void DMMessageHasDefaultCtor()
        {
            new DMMessage();
        }

        [TestMethod]
        public void TestSetGetData()
        {
            var msg = new DMMessage();
            msg.SetData("abc");
            var data = msg.GetDataString();
            Assert.AreEqual(data, "abc");
        }

        [TestMethod]
        public void TestWriteToStream()
        {
            var data = "abc";

            var msg = new DMMessage();
            msg.SetData(data);

            var stream = new InMemoryRandomAccessStream();

            DMMessage.WriteToStreamAsync(msg, stream).Wait();

            byte[] bytes = new byte[stream.Size];

            stream.Seek(0); // rewind to beginning

            stream.ReadAsync(bytes.AsBuffer(), (uint)bytes.Length, InputStreamOptions.None).AsTask().Wait();

            // The first 4 bytes is context, followed by data size, followed by the payload
            var contextSize = sizeof(UInt32);
            var dataSizeSize = sizeof(UInt32);
            var payloadSize = bytes.Length - contextSize - dataSizeSize;

            var contextValue = BitConverter.ToUInt32(bytes, 0);
            Assert.AreEqual<UInt32>(contextValue, 0U);

            var dataSizeValue = BitConverter.ToUInt32(bytes, contextSize);
            Assert.AreEqual<UInt32>(dataSizeValue, (UInt32)payloadSize);

            byte[] payload = new byte[payloadSize];
            Array.Copy(bytes, bytes.Length - payloadSize, payload, 0, payloadSize);

            var str = System.Text.Encoding.Unicode.GetString(payload);
            Assert.AreEqual(str, data);
        }

        [TestMethod]
        public void TestReadFromStream()
        {
            byte[] bytes = { 0, 0, 0, 0,
                            2, 0, 0, 0,
                            9, 9};

            var stream = new InMemoryRandomAccessStream();
            stream.WriteAsync(bytes.AsBuffer()).AsTask().Wait();
            stream.Seek(0); // rewind to beginning

            var message = DMMessage.ReadFromStreamAsync(stream).Result;

            Assert.AreEqual<UInt32>(message.Context, 0U);
            Assert.AreEqual<UInt32>((uint)message.Data.Length, 2U);
            Assert.AreEqual<UInt32>((uint)message.Data[0], 9);
            Assert.AreEqual<UInt32>((uint)message.Data[1], 9);
        }
    }
}
