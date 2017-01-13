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