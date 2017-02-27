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

            var processLauncherResult = await ProcessLauncher.RunToCompletionAsync(@"CommProxy.exe", "", processLauncherOptions);
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

        /*
        // ToDo: Fix: error CS1061: 'T' does not contain a definition for 'Status'.
        private void Wait<T>(T action, int milliSeconds)
        {
            while (action.Status != Windows.Foundation.AsyncStatus.Completed)
            {
                System.Threading.Tasks.Task.Delay(milliSeconds).Wait();
            }
        }
        */

        private void Wait(Windows.Foundation.IAsyncAction action, int milliSeconds)
        {
            while (action.Status != Windows.Foundation.AsyncStatus.Completed)
            {
                System.Threading.Tasks.Task.Delay(milliSeconds).Wait();
            }
        }

        private void Wait(Windows.Foundation.IAsyncOperation<ProcessLauncherResult> action, int milliSeconds)
        {
            while (action.Status != Windows.Foundation.AsyncStatus.Completed)
            {
                System.Threading.Tasks.Task.Delay(milliSeconds).Wait();
            }
        }

        private void Wait(Windows.Foundation.IAsyncOperation<Blob> action, int milliSeconds)
        {
            while (action.Status != Windows.Foundation.AsyncStatus.Completed)
            {
                System.Threading.Tasks.Task.Delay(milliSeconds).Wait();
            }
        }

        public IResponse SendCommand(IRequest command)
        {
            var processLauncherOptions = new ProcessLauncherOptions();
            var standardInput = new InMemoryRandomAccessStream();
            var standardOutput = new InMemoryRandomAccessStream();

            processLauncherOptions.StandardOutput = standardOutput;
            processLauncherOptions.StandardError = null;
            processLauncherOptions.StandardInput = standardInput.GetInputStreamAt(0);

            Windows.Foundation.IAsyncAction writeAction = command.Serialize().WriteToIOutputStreamAsync(standardInput);
            Wait(writeAction, 100);

            standardInput.Dispose();

            Windows.Foundation.IAsyncOperation<ProcessLauncherResult> result = ProcessLauncher.RunToCompletionAsync(@"CommProxy.exe", "", processLauncherOptions);
            Wait(result, 100);

            ProcessLauncherResult processLauncherResult = result.GetResults();
            if (processLauncherResult.ExitCode == 0)
            {
                using (var outStreamRedirect = standardOutput.GetInputStreamAt(0))
                {
                    Windows.Foundation.IAsyncOperation<Blob> readResult = Blob.ReadFromIInputStreamAsync(outStreamRedirect);
                    Wait(readResult, 100);

                    var response = readResult.GetResults().MakeIResponse();
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