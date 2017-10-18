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
        public void TestWifiConfigRequestResponseSerializeDeserialize()
        {
            // validate GetWifiConfigurationRequest serial/deserial round trip
            {
                var wifiConfigRequest = new GetWifiConfigurationRequest();
                var blob = wifiConfigRequest.Serialize();
                var wifiConfigRequestRehydrated = GetWifiConfigurationRequest.Deserialize(blob) as GetWifiConfigurationRequest;

                Assert.AreNotEqual(wifiConfigRequestRehydrated, null);
                Assert.AreEqual(wifiConfigRequestRehydrated.Tag, DMMessageKind.GetWifiConfiguration);
            }

            // validate SetWifiConfigurationRequest serial/deserial round trip with Apply='no'
            {
                var wifiConfigRequest = new SetWifiConfigurationRequest(
                    new WifiConfiguration()
                    {
                        Profiles = new List<WifiProfileConfiguration>() {
                            new WifiProfileConfiguration() { Name = "abc", Path = "ade", Uninstall = false, Xml = "<afg/>" },
                            new WifiProfileConfiguration() { Name = "lmn", Path = "lop", Uninstall = false, Xml = "<lqr/>" },
                            new WifiProfileConfiguration() { Name = "wxy", Path = "wzz", Uninstall = true, Xml = "<zzz/>" },
                        },
                        ApplyFromDeviceTwin = "no",
                    });
                var blob = wifiConfigRequest.Serialize();
                var wifiConfigRequestRehydrated = SetWifiConfigurationRequest.Deserialize(blob) as SetWifiConfigurationRequest;

                Assert.AreNotEqual(wifiConfigRequestRehydrated, null);
                Assert.AreEqual(wifiConfigRequestRehydrated.Tag, DMMessageKind.SetWifiConfiguration);
                Assert.AreNotEqual(wifiConfigRequestRehydrated.Configuration, null);
                Assert.AreNotEqual(wifiConfigRequestRehydrated.Configuration.Profiles, null);
                Assert.AreEqual(wifiConfigRequestRehydrated.Configuration.Profiles.Count, 0);
                Assert.AreEqual(wifiConfigRequestRehydrated.Configuration.ApplyFromDeviceTwin, "no");
            }

            // validate SetWifiConfigurationRequest serial/deserial round trip with Apply='yes'
            {
                var abc = new WifiProfileConfiguration() { Name = "abc", Path = "ade", Uninstall = false, Xml = "<afg/>" };
                var lmn = new WifiProfileConfiguration() { Name = "lmn", Path = "lop", Uninstall = false, Xml = "<lqr/>" };
                var wxy = new WifiProfileConfiguration() { Name = "wxy", Path = "wzz", Uninstall = true, Xml = "<zzz/>" };
                var wifiConfigRequest = new SetWifiConfigurationRequest(
                    new WifiConfiguration()
                    {
                        Profiles = new List<WifiProfileConfiguration>() { abc, lmn, wxy, },
                        ApplyFromDeviceTwin = "yes",
                    });
                var blob = wifiConfigRequest.Serialize();
                var wifiConfigRequestRehydrated = SetWifiConfigurationRequest.Deserialize(blob) as SetWifiConfigurationRequest;

                Assert.AreNotEqual(wifiConfigRequestRehydrated, null);
                Assert.AreEqual(wifiConfigRequestRehydrated.Tag, DMMessageKind.SetWifiConfiguration);
                Assert.AreNotEqual(wifiConfigRequestRehydrated.Configuration, null);
                Assert.AreNotEqual(wifiConfigRequestRehydrated.Configuration.Profiles, null);
                Assert.AreEqual(wifiConfigRequestRehydrated.Configuration.Profiles.Count, 3);
                Assert.AreEqual(wifiConfigRequestRehydrated.Configuration.ApplyFromDeviceTwin, "yes");

                foreach (var profile in wifiConfigRequestRehydrated.Configuration.Profiles)
                {
                    WifiProfileConfiguration compare = null;
                    if (profile.Name == "abc") compare = abc;
                    else if (profile.Name == "lmn") compare = lmn;
                    else if (profile.Name == "wxy") compare = wxy;
                    else Assert.Fail("unknown wifi profile name");

                    Assert.AreEqual(profile.Name, compare.Name);
                    Assert.AreEqual(profile.Uninstall, compare.Uninstall);
                    if (profile.Uninstall)
                    {
                        Assert.AreEqual(profile.Path, "");
                        Assert.AreEqual(profile.Xml, "");
                    }
                    else
                    {
                        Assert.AreEqual(profile.Path, compare.Path);
                        Assert.AreEqual(profile.Xml, compare.Xml);
                    }
                }
            }

            // validate GetWifiConfigurationResponse serial/deserial round trip w/ Report='no'
            {
                //
                // When Report=no, no profiles are serialized
                //
                var wifiConfigResponse = new GetWifiConfigurationResponse(
                    ResponseStatus.Success,
                    new WifiConfiguration()
                    {
                        Profiles = new List<WifiProfileConfiguration>() {
                            new WifiProfileConfiguration() { Name = "foo", Path = "bar", Uninstall = false, Xml = "<abc/>" },
                        },
                        ReportToDeviceTwin = "no",
                    });
                var blob = wifiConfigResponse.Serialize();
                var wifiConfigResponseRehydrated = GetWifiConfigurationResponse.Deserialize(blob) as GetWifiConfigurationResponse;

                Assert.AreNotEqual(wifiConfigResponseRehydrated, null);
                Assert.AreEqual(wifiConfigResponseRehydrated.Tag, DMMessageKind.GetWifiConfiguration);
                Assert.AreNotEqual(wifiConfigResponseRehydrated.Configuration, null);
                Assert.AreNotEqual(wifiConfigResponseRehydrated.Configuration.Profiles, null);
                Assert.AreEqual(wifiConfigResponseRehydrated.Configuration.Profiles.Count, 0);
                Assert.AreEqual(wifiConfigResponseRehydrated.Configuration.ReportToDeviceTwin, "no");
            }

            // validate GetWifiConfigurationResponse serial/deserial round trip
            {
                //
                // GetWifiConfiguration only returns name, reflecting which profiles are installed ... none of 
                // the other details are serialized.
                //
                var wifiConfigResponse = new GetWifiConfigurationResponse(
                    ResponseStatus.Success,
                    new WifiConfiguration() {
                        Profiles = new List<WifiProfileConfiguration>() {
                            new WifiProfileConfiguration() { Name = "foo", Path = "bar", Uninstall = false, Xml = "<abc/>" },
                        },
                        ReportToDeviceTwin = "yes",
                    });
                var blob = wifiConfigResponse.Serialize();
                var wifiConfigResponseRehydrated = GetWifiConfigurationResponse.Deserialize(blob) as GetWifiConfigurationResponse;

                Assert.AreNotEqual(wifiConfigResponseRehydrated, null);
                Assert.AreEqual(wifiConfigResponseRehydrated.Tag, DMMessageKind.GetWifiConfiguration);
                Assert.AreNotEqual(wifiConfigResponseRehydrated.Configuration, null);
                Assert.AreNotEqual(wifiConfigResponseRehydrated.Configuration.Profiles, null);
                Assert.AreEqual(wifiConfigResponseRehydrated.Configuration.Profiles.Count, 1);
                var foo = wifiConfigResponseRehydrated.Configuration.Profiles[0];
                Assert.AreEqual(foo.Name, "foo");
                Assert.AreEqual(foo.Path, "");
                Assert.AreEqual(foo.Uninstall, false);
                Assert.AreEqual(foo.Xml, "");
            }
        }

        [TestMethod]
        public void TestRequestSerializeDeserialize()
        {
            var appInstallRequest = new AppInstallRequest(new AppInstallRequestData() { AppxPath = "abc", PackageFamilyName = "def", Dependencies = new List<String>() { "ghi", "jkl" } });
            var blob = appInstallRequest.Serialize();
            var appInstallRequestRehydrated = AppInstallRequest.Deserialize(blob) as AppInstallRequest;

            Assert.AreEqual(appInstallRequestRehydrated.Tag, DMMessageKind.InstallApp);
            Assert.AreEqual(appInstallRequestRehydrated.data.AppxPath, "abc");
            Assert.AreEqual(appInstallRequestRehydrated.data.PackageFamilyName, "def");
            Assert.AreEqual(appInstallRequestRehydrated.data.Dependencies[0], "ghi");
            Assert.AreEqual(appInstallRequestRehydrated.data.Dependencies[1], "jkl");
        }

        [TestMethod]
        public void TestReadFromIInputStream()
        {
            var command = new AppInstallRequest(new AppInstallRequestData() { AppxPath = "abc", PackageFamilyName = "def", Dependencies = new List<String>() { "ghi", "jkl" } });
            var dataArray = command.Serialize().GetByteArrayForSerialization();
            var dataSizeArray = BitConverter.GetBytes((UInt32)dataArray.Length);

            var stream = new InMemoryRandomAccessStream();
            stream.WriteAsync(dataSizeArray.AsBuffer()).AsTask().Wait();
            stream.WriteAsync(dataArray.AsBuffer()).AsTask().Wait();

            var result = Blob.ReadFromIInputStreamAsync(stream.GetInputStreamAt(0)).AsTask().Result;

            Assert.AreEqual(result.Tag, DMMessageKind.InstallApp);

            var request = result.MakeIRequest() as AppInstallRequest;

            Assert.AreEqual(request.Tag, DMMessageKind.InstallApp);
            Assert.AreEqual(request.data.AppxPath, "abc");
            Assert.AreEqual(request.data.PackageFamilyName, "def");
            Assert.AreEqual(request.data.Dependencies[0], "ghi");
            Assert.AreEqual(request.data.Dependencies[1], "jkl");
        }

        [TestMethod]
        public void TestWriteToOutputStream()
        {
            var command = new AppInstallRequest(new AppInstallRequestData() { AppxPath = "abc", PackageFamilyName = "def", Dependencies = new List<String>() { "ghi", "jkl" } });
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
            Assert.AreEqual(command2.data.AppxPath, "abc");
            Assert.AreEqual(command2.data.PackageFamilyName, "def");
            Assert.AreEqual(command2.data.Dependencies[0], "ghi");
            Assert.AreEqual(command2.data.Dependencies[1], "jkl");
        }


        [TestMethod]
        public void TestSerializationRoundtripThroughStream()
        {
            var command = new AppInstallRequest(new AppInstallRequestData() { AppxPath = "abc", PackageFamilyName = "def", Dependencies = new List<String>() { "ghi", "jkl" } });
            var blob = command.Serialize();

            var stream = new InMemoryRandomAccessStream();

            blob.WriteToIOutputStreamAsync(stream.GetOutputStreamAt(0)).AsTask().Wait();
            var blob2 = Blob.ReadFromIInputStreamAsync(stream.GetInputStreamAt(0)).AsTask().Result;

            Assert.AreEqual(blob2.Tag, blob.Tag);

            var command2 = blob2.MakeIRequest() as AppInstallRequest;

            Assert.IsNotNull(command2);
            Assert.AreEqual(command2.Tag, DMMessageKind.InstallApp);
            Assert.AreEqual(command2.data.AppxPath, "abc");
            Assert.AreEqual(command2.data.PackageFamilyName, "def");
            Assert.AreEqual(command2.data.Dependencies[0], "ghi");
            Assert.AreEqual(command2.data.Dependencies[1], "jkl");

        }

        [TestMethod]
        public void TestRequestSendToProxy()
        {
            var appInstallRequest = new AppInstallRequest(new AppInstallRequestData() { AppxPath = "abc", PackageFamilyName = "def", Dependencies = new List<String>() { "ghi", "jkl" } });
            var proxy = new ConfigurationProxyMockup();

            IResponse response = proxy.SendCommandAsync(appInstallRequest).Result;
            Assert.AreEqual(response.Status, ResponseStatus.Success);
        }

    }
}
