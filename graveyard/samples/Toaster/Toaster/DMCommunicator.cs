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
using Microsoft.VisualStudio.Threading;
using System;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Text;
using System.Threading.Tasks;
using Windows.ApplicationModel;
using Windows.Foundation;
using Windows.Storage.Streams;
using Windows.System;

namespace Toaster
{
    class Message
    {
        public string Payload;
    }

    class DMCommunicator
    {
        private static readonly Lazy<DMCommunicator> _instance = new Lazy<DMCommunicator>(() => new DMCommunicator());
        private Action<Message> _callback;

        IAsyncOperation<ProcessLauncherResult> processLauncherResult;
        ProcessLauncherOptions processLauncherOptions;
        IRandomAccessStream standardInput;

        private void StartProxyProcess()
        {
            processLauncherOptions = new ProcessLauncherOptions();
            standardInput = new InMemoryRandomAccessStream();

            processLauncherOptions.StandardOutput = null;
            processLauncherOptions.StandardError = null;
            processLauncherOptions.StandardInput = standardInput.GetInputStreamAt(0);

            processLauncherResult = ProcessLauncher.RunToCompletionAsync(
                "comm-proxy.exe",
                "com.microsoft.iot-dm" + " " + Package.Current.Id.FamilyName,
                processLauncherOptions);
        }

        private DMCommunicator()
        {
            _callback = (message) =>
            {
                System.Diagnostics.Debug.WriteLine("Warning: callback not set");
            };

            StartProxyProcess();
        }

        public static DMCommunicator Instance
        {
            get
            {
                return _instance.Value;
            }
        }

        public void SendMessage(Message message)
        {
            byte[] buffer = Encoding.ASCII.GetBytes(message.Payload);
            IBuffer ibuffer = buffer.AsBuffer();
            standardInput.WriteAsync(ibuffer).AsTask().Forget();
        }

        public Action<Message> MessageReceivedCallback
        {
            get { return _callback; }
            set { _callback = value; }
        }

    }
}
