using System;
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Text;
using System.Threading.Tasks;
using Windows.ApplicationModel;
using Windows.Foundation;
using Windows.Storage.Streams;
using Windows.System;
using System.Diagnostics;

namespace Microsoft.Devices.Management
{
    // This must be kept in sync with enum class dm_command in dm_request.h
    public enum DMCommand
    {
        Unknown = 0,
        FactoryReset = 1,
        CheckUpdates = 2,

        // Reboot
        RebootSystem = 10,
        SetSingleRebootTime = 11,
        GetSingleRebootTime = 12,
        SetDailyRebootTime = 13,
        GetDailyRebootTime = 14,
        GetLastRebootCmdTime = 15,
        GetLastRebootTime = 16,
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

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode, Pack = 1)]
    unsafe struct DMResponse
    {
        public UInt32 status;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
        public string message;
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

        private static DMResponse Deserialize(ref byte[] serializedData)
        {
            GCHandle gch = GCHandle.Alloc(serializedData, GCHandleType.Pinned);
            IntPtr pbyteSerializedData = gch.AddrOfPinnedObject();
            var result = (DMResponse)Marshal.PtrToStructure<DMResponse>(pbyteSerializedData);
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
                    uint size = (uint)standardOutput.Size;
                    System.Diagnostics.Debug.WriteLine(string.Format("Received {0} bytes from comm-proxy", size));

                    byte[] bytes = new byte[size];
                    IBuffer ibuffer = bytes.AsBuffer();
                    var result = await outStreamRedirect.ReadAsync(ibuffer, size, InputStreamOptions.None);
                    var response = Deserialize(ref bytes);
                    return response;
                }
            }
            else
            {
                // TODO: handle error
                var response = new DMResponse();
                response.status = 500;
                return response;
            }
        }
    }
}