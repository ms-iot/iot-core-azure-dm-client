using System;
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Text;
using System.Threading.Tasks;
using Windows.ApplicationModel;
using Windows.Foundation;
using Windows.Storage.Streams;
using Windows.System;

namespace Microsoft.Devices.Management
{
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    unsafe struct dm_request
    {
        public UInt32 command;
        public fixed byte data[64];
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode, Pack = 1)]
    unsafe struct dm_response
    {
        public UInt32 status;
        public fixed byte data[64];

        // Optional:
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
        public string message;
    }

    // This class send requests (dm_request) to the System Configurator and receives the responses (dm_response) from it
    static class SystemConfiguratorProxy
    {
        private static byte[] Serialize(dm_request command)
        {
            Int32 size = Marshal.SizeOf<dm_request>();
            byte[] bytes = new byte[size];
            GCHandle gch = GCHandle.Alloc(bytes, GCHandleType.Pinned);
            IntPtr pbyteArrayMyDataStruct = gch.AddrOfPinnedObject();
            Marshal.StructureToPtr(command, pbyteArrayMyDataStruct, false);
            gch.Free();
            return bytes;
        }

        private static dm_response Deserialize(ref byte[] serializedData)
        {
            GCHandle gch = GCHandle.Alloc(serializedData, GCHandleType.Pinned);
            IntPtr pbyteSerializedData = gch.AddrOfPinnedObject();
            var result = (dm_response)Marshal.PtrToStructure<dm_response>(pbyteSerializedData);
            gch.Free();
            return result;
        }

        public static async Task<dm_response> SendCommandAsync(dm_request command)
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

            var processLauncherResult = await ProcessLauncher.RunToCompletionAsync("comm-proxy.exe", "", processLauncherOptions);
            if (processLauncherResult.ExitCode == 0)
            {
                using (var outStreamRedirect = standardOutput.GetInputStreamAt(0))
                {
                    uint size = (uint)standardOutput.Size;

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
                var response = new dm_response();
                response.status = 500;
                return response;
            }
        }
    }
}