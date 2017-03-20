// #define DEBUG_COMMPROXY_OUTPUT

using System;
using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Runtime.Serialization;
using System.Threading.Tasks;
using Microsoft.Devices.Management.Message;
using Windows.Storage.Streams;
using Windows.System;

namespace Microsoft.Devices.Management
{

    // This class send requests (DMrequest) to the System Configurator and receives the responses (DMesponse) from it
    class SystemConfiguratorProxy : ISystemConfiguratorProxy
    {
        public async Task<IResponse> SendCommandAsync(IRequest command)
        {
            var processLauncherOptions = new ProcessLauncherOptions();
            var standardInput = new InMemoryRandomAccessStream();
            var standardOutput = new InMemoryRandomAccessStream();

            processLauncherOptions.StandardOutput = standardOutput;
            processLauncherOptions.StandardError = null;
            processLauncherOptions.StandardInput = standardInput.GetInputStreamAt(0);

            await command.Serialize().WriteToIOutputStreamAsync(standardInput);

            standardInput.Dispose();

            var processLauncherResult = await ProcessLauncher.RunToCompletionAsync(@"C:\Windows\System32\CommProxy.exe", "", processLauncherOptions);
            if (processLauncherResult.ExitCode == 0)
            {
                using (var outStreamRedirect = standardOutput.GetInputStreamAt(0))
                {
                    var response = (await Blob.ReadFromIInputStreamAsync(outStreamRedirect)).MakeIResponse();
                    if (response.Status != ResponseStatus.Success)
                    {
                        var stringResponse = response as StringResponse;
                        if (stringResponse != null) throw new Exception(stringResponse.Response);
                        throw new Exception("Operation failed");
                    }
                    return response;
                }
            }
            else
            {
                throw new Exception("CommProxy cannot read data from the input pipe");
            }
        }
    }
}