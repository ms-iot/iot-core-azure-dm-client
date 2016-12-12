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
using System.Collections.Generic;

namespace Microsoft.Devices.Management
{
    // This must be kept in sync with enum class dm_command in dm_request.h
    public enum DMCommand
    {
        SystemReboot = 1,
        FactoryReset = 2,
        CheckUpdates = 3,
        ListApps = 4
    }

    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    unsafe struct DMRequest
    {
        [MarshalAs(UnmanagedType.U4)]
        public DMCommand command;
        public fixed byte data[64];
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode, Pack = 1)]
    unsafe struct DMResponse
    {
        public UInt32 status;
        public fixed byte data[64];

        // Optional:
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 1024)]
        public string message;

        public UInt32 chunkIndex;
        public UInt32 chunkCount;
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

        public static async Task<List<DMResponse>> SendCommandAsync(DMRequest command)
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
                    var size = (uint)standardOutput.Size;
                    System.Diagnostics.Debug.WriteLine(string.Format("Received {0} bytes from comm-proxy", size));

                    var iStructSize = Marshal.SizeOf<DMResponse>();
                    var responses = new List<DMResponse>();

                    var bytes = new byte[iStructSize];
                    var ibuffer = bytes.AsBuffer();
                    var corruptionCheck = 0;
                    while (true)
                    {
                        var result = await outStreamRedirect.ReadAsync(ibuffer, (uint)iStructSize, InputStreamOptions.None);
                        var response = Deserialize(ref bytes);
                        if (corruptionCheck != response.chunkIndex)
                        {
                            break;
                        }
                        responses.Add(response);
                        if (response.chunkIndex >= (response.chunkCount - 1))
                        {
                            break;
                        }
                        corruptionCheck++;
                    }
                    return responses;
                }
            }
            else
            {
                // TODO: handle error
                var response = new DMResponse();
                response.status = 500;
                return new List<DMResponse>() { response };
            }
        }
    }
}