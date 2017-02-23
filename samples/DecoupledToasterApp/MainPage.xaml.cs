using Microsoft.Azure.Devices.Shared;
using Newtonsoft.Json;
using System;
using System.Threading.Tasks;
using Windows.ApplicationModel.AppService;
using Windows.Foundation.Collections;
using Windows.UI.Core;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;

namespace Toaster
{
    public sealed partial class MainPage : Page
    {
        public MainPage()
        {
            this.InitializeComponent();
            this.buttonStart.IsEnabled = true;
            this.buttonStop.IsEnabled = false;
            this.imageHot.Visibility = Visibility.Collapsed;

            this.Loaded += MainPage_Loaded;
        }

        private AppServiceConnection _connection;
        private async void MainPage_Loaded(object sender, RoutedEventArgs e)
        {
            await CreateConnection();
        }

        private Task CreateConnection()
        {
            _connection = new AppServiceConnection();
            _connection.AppServiceName = "DMClientAppService";
            _connection.PackageFamilyName = "23983CETAthensQuality.IoTDMBackgroundSample_1w720vyc4ccym";
            var openTask = _connection.OpenAsync();
            openTask.AsTask().Wait();
            var status = openTask.GetResults();
            if (status != AppServiceConnectionStatus.Success)
            {
                // wait and try again?
            }
            else
            {
                _connection.RequestReceived += Connection_RequestReceived;
                _connection.ServiceClosed += Connection_ServiceClosed;
            }

            return Task.CompletedTask;
        }
        private async void Connection_ServiceClosed(AppServiceConnection sender, AppServiceClosedEventArgs args)
        {
            await CreateConnection();
        }

        private async Task<string> SendMessageToDmClient(string name, string value)
        {
            var request = new ValueSet();
            request.Add(name, value);
            var response = await _connection.SendMessageAsync(request);
            if (response.Status == AppServiceResponseStatus.Success)
            {
                return response.Message["returnValue"] as string;
            }
            return string.Empty;
        }

        private async void Connection_RequestReceived(AppServiceConnection sender, AppServiceRequestReceivedEventArgs args)
        {
            var deferral = args.GetDeferral();
            ValueSet message = args.Request.Message;
            if (message.ContainsKey("desiredPropertyUpdate"))
            {
                string jsonProperties = message["desiredPropertyUpdate"] as string;
                var desiredProperties = JsonConvert.DeserializeObject<TwinCollection>(jsonProperties);
                await OnDesiredPropertyUpdate(desiredProperties, this);
            }
            else if (message.ContainsKey("operation"))
            {
                if (message["operation"].Equals("readyForReboot"))
                {
                    var ready = await YesNo("Allow reboot?");
                    var response = new ValueSet();
                    response.Add("readyForReboot", ready);
                    await sender.SendMessageAsync(response);
                }
            }
            deferral.Complete();
        }

        public Task OnDesiredPropertyUpdate(TwinCollection desiredProperties, object userContext)
        {
            // Application developer can process all the top-level nodes here
            return Task.CompletedTask;
        }

        // This method may get called on the DM callback thread - not on the UI thread.
        public async Task<bool> YesNo(string question)
        {
            var tcs = new TaskCompletionSource<bool>();

            await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, async () =>
            {
                UserDialog dlg = new UserDialog(question);
                ContentDialogResult dialogResult = await dlg.ShowAsync();
                tcs.SetResult(dlg.Result);
            });

            bool yesNo = await tcs.Task;
            return yesNo;
        }

        private void OnStartToasting(object sender, RoutedEventArgs e)
        {
            this.buttonStart.IsEnabled = false;
            this.buttonStop.IsEnabled = true;
            this.slider.IsEnabled = false;
            this.textBlock.Text = string.Format("Toasting at {0}%", this.slider.Value);
            this.imageHot.Visibility = Visibility.Visible;
        }

        private void OnStopToasting(object sender, RoutedEventArgs e)
        {
            this.buttonStart.IsEnabled = true;
            this.buttonStop.IsEnabled = false;
            this.slider.IsEnabled = true;
            this.textBlock.Text = "Ready";
            this.imageHot.Visibility = Visibility.Collapsed;
        }

        private async void OnCheckForUpdates(object sender, RoutedEventArgs e)
        {
            bool updatesAvailable = (await SendMessageToDmClient("request", "checkForUpdates")).Equals(true.ToString());
            if (updatesAvailable)
            {
                System.Diagnostics.Debug.WriteLine("updates available");
                var dlg = new UserDialog("Updates available. Install?");
                await dlg.ShowAsync();
                // Don't do anything yet
            }
        }

        private async void RestartSystem()
        {
            bool success = (await SendMessageToDmClient("request", "immediateReboot")).Equals(true.ToString());
            StatusText.Text = success?  "Operation completed" : "Operation  failed";
        }

        private void OnSystemRestart(object sender, RoutedEventArgs e)
        {
            RestartSystem();
        }

        private async void FactoryReset()
        {
            bool success = (await SendMessageToDmClient("request", "factoryReset")).Equals(true.ToString());
            StatusText.Text = success? "Succeeded!" : "Failed!";
         }

        private void OnFactoryReset(object sender, RoutedEventArgs e)
        {
            FactoryReset();
        }
    }
}
