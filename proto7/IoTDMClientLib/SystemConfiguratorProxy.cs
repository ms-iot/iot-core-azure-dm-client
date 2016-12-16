// #define DEBUG_COMMPROXY_OUTPUT

using System;
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.WindowsRuntime;
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


    public class DMResponse
    {
        public UInt32 status;
        public byte[] data;

        internal DMResponse(UInt32 status, uint dataSize)
        {
            this.status = status;
            this.data = new byte[dataSize];
        }
        
        // GetDataString() assumes the strings received are unicode.
        public string GetDataString()
        {
            return System.Text.UnicodeEncoding.Unicode.GetString(data);
        }
    }

    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    unsafe struct DMRequest
    {
        const int DataSize = 64;

        [MarshalAs(UnmanagedType.U4)]
        public DMCommand command;

        public fixed byte data[DataSize];

        public unsafe void SetData(string valueString)
        {
            if (valueString.Length > DataSize - 1)
            {
                throw new ArgumentException();
            }

            byte[] stringBytes = System.Text.ASCIIEncoding.ASCII.GetBytes(valueString);

            unsafe
            {
                fixed (byte* dataBytes = data)
                {
                    int i = 0;
                    for (; i < valueString.Length; ++i)
                    {
                        dataBytes[i] = stringBytes[i];
                    }
                    for (; i < DataSize; ++i)
                    {
                        dataBytes[i] = 0;
                    }
                }
            }
        }
    }

    // This class send requests (DMrequest) to the System Configurator and receives the responses (DMesponse) from it
    static class SystemConfiguratorProxy
    {
        private static byte[] Serialize(DMRequest command)
        {
            Int32 size = Marshal.SizeOf<DMRequest>();
            byte[] bytes = new byte[size];
            GCHandle gch = GCHandle.Alloc(bytes, GCHandleType.Pinned);
            IntPtr pbyteArrayMyDataStruct = gch.AddrOfPinnedObject();
            Marshal.StructureToPtr(command, pbyteArrayMyDataStruct, false);
            gch.Free();
            return bytes;
        }

        private static T Deserialize<T>(ref byte[] serializedData)
        {
            GCHandle gch = GCHandle.Alloc(serializedData, GCHandleType.Pinned);
            IntPtr pbyteSerializedData = gch.AddrOfPinnedObject();
            var result = (T)Marshal.PtrToStructure<T>(pbyteSerializedData);
            gch.Free();
            return result;
        }

        public static async Task<DMResponse> SendCommandAsync(DMRequest command)
        {
            var processLauncherOptions = new ProcessLauncherOptions();
            var standardInput = new InMemoryRandomAccessStream();
            var standardOutput = new InMemoryRandomAccessStream();

            processLauncherOptions.StandardOutput = standardOutput;
            processLauncherOptions.StandardError = null;
            processLauncherOptions.StandardInput = standardInput.GetInputStreamAt(0);

            var input_buffer = Serialize(command);
            await standardInput.WriteAsync(input_buffer.AsBuffer());
            await standardInput.FlushAsync();
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
                    var uint32Size = Marshal.SizeOf<UInt32>();
                    var uint32Bytes = new byte[uint32Size];
                    var uint32Buffer = uint32Bytes.AsBuffer();
                    // read the status
                    var statusResult = await outStreamRedirect.ReadAsync(uint32Buffer, (uint)uint32Size, InputStreamOptions.None);
                    var status = Deserialize<UInt32>(ref uint32Bytes);
                    // read the dataSize
                    var dataSizeResult = await outStreamRedirect.ReadAsync(uint32Buffer, (uint)uint32Size, InputStreamOptions.None);
                    var dataSize = Deserialize<UInt32>(ref uint32Bytes);
                    var response = new DMResponse(status, dataSize);
                    // read the data if needed
                    if (dataSize != 0)
                    {
                        var dataBuffer = response.data.AsBuffer();
                        var dataResult = await outStreamRedirect.ReadAsync(dataBuffer, (uint)dataSize, InputStreamOptions.None);
                    }
                    return response;
#endif
                }
            }
            else
            {
                // TODO: handle error
                return new DMResponse(500, 0);
            }
        }
    }
}