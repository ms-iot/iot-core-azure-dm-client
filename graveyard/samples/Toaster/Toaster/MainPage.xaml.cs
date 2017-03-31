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
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Threading.Tasks;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;
using Windows.System;
using Windows.Storage.Streams;
using Windows.ApplicationModel;
using System.Text;
using Windows.ApplicationModel.Core;
using Windows.UI.Core;
using Microsoft.VisualStudio.Threading;
using Newtonsoft.Json;

namespace Toaster
{
    public class DMResponse
    {
        public string Status;
        public int Power;
    }

/// <summary>
/// An empty page that can be used on its own or navigated to within a Frame.
/// </summary>
public sealed partial class MainPage : Page
    {
        public MainPage()
        {
            this.InitializeComponent();
            this.buttonStart.IsEnabled = true;
            this.buttonStop.IsEnabled = false;
            this.imageHot.Visibility = Visibility.Collapsed;

            DMCommunicator.Instance.MessageReceivedCallback = (message) =>
            {
                CoreApplication.MainView.CoreWindow.Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
                    {
                        var response = JsonConvert.DeserializeObject<DMResponse>(message.Payload);
                        this.textBlock.Text = string.Format("{0} at {1}%", response.Status, response.Power);
                    }).AsTask().Forget();
            };

            // YesNo("Allow Reboot?");
        }

        async Task<bool> YesNo(string question)
        {
            var dlg = new UserDialog(question);
            await dlg.ShowAsync();

            return dlg.Result;
        }

        private void buttonStart_Click(object sender, RoutedEventArgs e)
        {
            this.buttonStart.IsEnabled = false;
            this.buttonStop.IsEnabled = true;
            this.slider.IsEnabled = false;
            this.textBlock.Text = string.Format("Toasting at {0}%", this.slider.Value);
            this.imageHot.Visibility = Visibility.Visible;

            var msg = new
            {
                TimeStamp = DateTime.Now.ToString(),
                Command = "Start",
                Power = (int)this.slider.Value
            };

            var messageString = JsonConvert.SerializeObject(msg);

            SendDataToDM(messageString);

        }

        private void buttonStop_Click(object sender, RoutedEventArgs e)
        {
            this.buttonStart.IsEnabled = true;
            this.buttonStop.IsEnabled = false;
            this.slider.IsEnabled = true;
            this.textBlock.Text = "Ready";
            this.imageHot.Visibility = Visibility.Collapsed;
        }

        private void SendDataToDM(string data)
        {
            DMCommunicator.Instance.SendMessage(new Message { Payload = data });
        }

    }
}
