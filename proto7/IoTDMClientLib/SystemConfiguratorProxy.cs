// #define DEBUG_COMMPROXY_OUTPUT

using System;
using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Runtime.Serialization;
using System.Threading.Tasks;
using Windows.Storage.Streams;
using Windows.System;

namespace Microsoft.Devices.Management
{
    // This must be kept in sync with enum class dm_command in dm_request.h
    public enum DMCommand
    {
        Unknown = 0,
        FactoryReset = 1,
        CheckUpdates = 2,
        ListApps = 3,
        InstallApp = 4,
        UninstallApp = 5,

        // Reboot
        RebootSystem = 10,
        SetSingleRebootTime = 11,
        GetSingleRebootTime = 12,
        SetDailyRebootTime = 13,
        GetDailyRebootTime = 14,
        GetLastRebootCmdTime = 15,
        GetLastRebootTime = 16,
    }


    public class DMMessage
    {
        private UInt32 context;
        private byte[] data;

        public UInt32 Context { get { return context; } set { context = value; } }
        public byte[] Data { get { return data; } set { data = value; } }

        internal DMMessage(DMCommand cmd) : this((UInt32)cmd, 0)
        {
        }

        internal DMMessage() : this(DMCommand.Unknown)
        {
        }

        internal DMMessage(UInt32 ctxt, uint dataSize)
        {
            Context = ctxt;
            Data = new byte[dataSize];
        }

        public string GetDataString()
        {
            return System.Text.Encoding.Unicode.GetString(Data);
        }

        public void SetData(string valueString)
        {
            Data = System.Text.Encoding.Unicode.GetBytes(valueString);
        }
        public void SetContext(DMCommand ctxt)
        {
            Context = (UInt32)ctxt;
        }
        public void SetContext(UInt32 ctxt)
        {
            Context = (UInt32)ctxt;
        }

        public static async Task WriteToStreamAsync(DMMessage message, IOutputStream iostream)
        {
            List<byte> messageBytes = new List<byte>();

            // Context
            {
                messageBytes.AddRange(BitConverter.GetBytes(message.Context));
                System.Diagnostics.Debug.WriteLine(string.Format("Serializing context: {0}", message.context));
            }

            // DataCount
            UInt32 dataCount = (UInt32)message.Data.Length;
            {
                messageBytes.AddRange(BitConverter.GetBytes(dataCount));
                System.Diagnostics.Debug.WriteLine(string.Format("Serializing dataCount: {0}", dataCount));
            }

            // Data
            if (dataCount > 0)
            {
                messageBytes.AddRange(message.Data);
                System.Diagnostics.Debug.WriteLine(string.Format("Serializing data: {0}", message.GetDataString()));
            }

            var commandResult = await iostream.WriteAsync(messageBytes.ToArray().AsBuffer());
            await iostream.FlushAsync();
        }

        private static T Deserialize<T>(ref byte[] serializedData)
        {
            GCHandle gch = GCHandle.Alloc(serializedData, GCHandleType.Pinned);
            IntPtr pbyteSerializedData = gch.AddrOfPinnedObject();
            var result = (T)Marshal.PtrToStructure<T>(pbyteSerializedData);
            gch.Free();
            return result;
        }

        public static async Task<DMMessage> ReadFromStreamAsync(IInputStream iistream)
        {
            var uint32Size = Marshal.SizeOf<UInt32>();
            var uint32Bytes = new byte[uint32Size];
            var uint32Buffer = uint32Bytes.AsBuffer();
            // read the status
            var statusResult = await iistream.ReadAsync(uint32Buffer, (uint)uint32Size, InputStreamOptions.None);
            var status = Deserialize<UInt32>(ref uint32Bytes);
            // read the dataSize
            var readDataSizeResult = await iistream.ReadAsync(uint32Buffer, (uint)uint32Size, InputStreamOptions.None);
            var dataSize = Deserialize<UInt32>(ref uint32Bytes);
            var response = new DMMessage(status, dataSize);
            // read the data if needed
            if (dataSize != 0)
            {
                var dataBuffer = response.data.AsBuffer();
                var dataResult = await iistream.ReadAsync(dataBuffer, (uint)dataSize, InputStreamOptions.None);
            }
            return response;
        }
    }

    // This class send requests (DMrequest) to the System Configurator and receives the responses (DMesponse) from it
    static class SystemConfiguratorProxy
    {
        public static async Task<DMMessage> SendCommandAsync(DMMessage command)
        {
            var processLauncherOptions = new ProcessLauncherOptions();
            var standardInput = new InMemoryRandomAccessStream();
            var standardOutput = new InMemoryRandomAccessStream();

            processLauncherOptions.StandardOutput = standardOutput;
            processLauncherOptions.StandardError = null;
            processLauncherOptions.StandardInput = standardInput.GetInputStreamAt(0);

            await DMMessage.WriteToStreamAsync(command, standardInput);
            standardInput.Dispose();

            var processLauncherResult = await ProcessLauncher.RunToCompletionAsync(@"CommProxy.exe", "", processLauncherOptions);
            if (processLauncherResult.ExitCode == 0)
            {
                using (var outStreamRedirect = standardOutput.GetInputStreamAt(0))
                {
#if DEBUG_COMMPROXY_OUTPUT
                    var size = (uint)standardOutput.Size;
                    System.Diagnostics.Debug.WriteLine(string.Format("Received {0} bytes from comm-proxy", size));
                    var bytes = new byte[size];
                    var ibuffer = bytes.AsBuffer();
                    var result = await outStreamRedirect.ReadAsync(ibuffer, (uint)size, InputStreamOptions.None);
                    string data = System.Text.Encoding.UTF8.GetString(bytes);
                    return new DMResponse(500, 0);
#else
                    var response = await DMMessage.ReadFromStreamAsync(outStreamRedirect);
                    return response;
#endif
                }
            }
            else
            {
                // TODO: handle error
                return new DMMessage(500, 0);
            }
        }
    }
}