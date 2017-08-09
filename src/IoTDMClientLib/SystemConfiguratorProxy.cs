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
// #define DEBUG_COMMPROXY_OUTPUT

using System;
using System.Diagnostics;
using System.Threading.Tasks;
using Microsoft.Devices.Management.Message;
using Windows.Foundation;
using Windows.Foundation.Diagnostics;
using Windows.Storage.Streams;
using Windows.System;
using Windows.UI.Core;

namespace Microsoft.Devices.Management
{

    // This class send requests (DMrequest) to the System Configurator and receives the responses (DMesponse) from it
    class SystemConfiguratorProxy : ISystemConfiguratorProxy
    {
        const string CommProxyExe = @"C:\Windows\System32\CommProxy.exe";
        const string CommProxyArgs = "";

        private void Wait(Func<bool> condition, string message)
        {
            while (condition())
            {
                Debug.WriteLine(message);
                CoreWindow coreWindow = CoreWindow.GetForCurrentThread();
                if (coreWindow != null)
                {
                    Debug.WriteLine("Processing events...");
                    coreWindow.Dispatcher.ProcessEvents(CoreProcessEventsOption.ProcessAllIfPresent);
                }
                else
                {
                    Debug.WriteLine("Sleeping...");
                    Task.Delay(200);
                }
            }
        }

        private void ThrowError(IResponse response)
        {
            if (response == null)
            {
                throw new Error(ErrorSubSystem.Unknown, -1, "SystemConfigurator returned a null response.");
            }
            else if (response is ErrorResponse)
            {
                var errorResponse = response as ErrorResponse;
                string message = "Sub-system=" + errorResponse.SubSystem.ToString() + ", code=" + errorResponse.ErrorCode + ", messag=" + errorResponse.ErrorMessage;
                Logger.Log(message, LoggingLevel.Error);
                Debug.WriteLine(message);
                throw new Error(errorResponse.SubSystem, errorResponse.ErrorCode, errorResponse.ErrorMessage);
            }
            else if (response is StringResponse)
            {
                var stringResponse = response as StringResponse;
                string message = "Error Tag(" + stringResponse.Tag.ToString() + ") : " + stringResponse.Status.ToString() + " : " + stringResponse.Response;
                Logger.Log(message, LoggingLevel.Error);
                Debug.WriteLine(message);
                throw new Error(ErrorSubSystem.Unknown, -1, message);
            }
        }

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

            var processLauncherResult = await ProcessLauncher.RunToCompletionAsync(CommProxyExe, CommProxyArgs, processLauncherOptions);
            if (processLauncherResult.ExitCode == 0)
            {
                using (var outStreamRedirect = standardOutput.GetInputStreamAt(0))
                {
                    var response = (await Blob.ReadFromIInputStreamAsync(outStreamRedirect)).MakeIResponse();
                    if (response.Status != ResponseStatus.Success)
                    {
                        ThrowError(response);
                    }
                    return response;
                }
            }
            else
            {
                throw new Exception("CommProxy cannot read data from the input pipe");
            }
        }

        public Task<IResponse> SendCommand(IRequest command)
        {
            var processLauncherOptions = new ProcessLauncherOptions();
            var standardInput = new InMemoryRandomAccessStream();
            var standardOutput = new InMemoryRandomAccessStream();

            processLauncherOptions.StandardOutput = standardOutput;
            processLauncherOptions.StandardError = null;
            processLauncherOptions.StandardInput = standardInput.GetInputStreamAt(0);

            var writeAsyncAction = command.Serialize().WriteToIOutputStreamAsync(standardInput);
            Wait(() => writeAsyncAction.Status == AsyncStatus.Started, "Waiting to finish writing to output stream...");
            standardInput.Dispose();

            var runAsyncAction = ProcessLauncher.RunToCompletionAsync(CommProxyExe, CommProxyArgs, processLauncherOptions);
            Wait(() => runAsyncAction.Status == AsyncStatus.Started, "Waiting for CommProxy.exe to finish...");

            ProcessLauncherResult processLauncherResult = runAsyncAction.GetResults();
            if (processLauncherResult.ExitCode == 0)
            {
                using (var outStreamRedirect = standardOutput.GetInputStreamAt(0))
                {
                    var readAsyncAction = Blob.ReadFromIInputStreamAsync(outStreamRedirect);
                    Wait(() => readAsyncAction.Status == AsyncStatus.Started, "Waiting to finish reading from input stream...");

                    var response = readAsyncAction.GetResults().MakeIResponse();
                    if (response.Status != ResponseStatus.Success)
                    {
                        ThrowError(response);
                    }
                    return Task.FromResult<IResponse>(response);
                }
            }
            else
            {
                throw new Exception("CommProxy cannot read data from the input pipe");
            }
        }
    }
}
