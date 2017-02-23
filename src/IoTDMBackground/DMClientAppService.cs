using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net.Http;
using Windows.ApplicationModel.Background;
using Microsoft.Azure.Devices.Client;
using Microsoft.Devices.Management;
using System.Threading.Tasks;
using Windows.ApplicationModel;
using Microsoft.Azure.Devices.Shared;
using Windows.ApplicationModel.AppService;
using Windows.Foundation.Collections;

// The Background Application template is documented at http://go.microsoft.com/fwlink/?LinkID=533884&clcid=0x409

namespace IoTDMBackground
{
    public sealed class DMClientAppService : IBackgroundTask
    {
        private BackgroundTaskDeferral _deferral;

        private AppServiceConnection _appServiceConnection;

        public void Run(IBackgroundTaskInstance taskInstance)
        {
            _deferral = taskInstance.GetDeferral();

            var appServiceTrigger = taskInstance.TriggerDetails as AppServiceTriggerDetails;
            if (appServiceTrigger != null)
            {
                if (appServiceTrigger.Name.Equals("DMClientAppService"))
                {
                    var connection = appServiceTrigger.AppServiceConnection;
                    _appServiceConnection.RequestReceived += AppServiceConnection_RequestReceived;
                    _appServiceConnection.ServiceClosed += AppServiceConnection_ServiceClosed;

                    //
                    // store so that the background application can initiate communication with
                    // any clients
                    //
                    DMClientBackgroundApplication.Instance.Connections.Add(appServiceTrigger.AppServiceConnection);
                }
                else
                {
                    _deferral.Complete();
                }
            }
        }

        //
        // Hold onto connection as long as it is needed
        //
        private async void AppServiceConnection_RequestReceived(AppServiceConnection sender, AppServiceRequestReceivedEventArgs args)
        {
            var deferral = args.GetDeferral();

            var requestMessage = args.Request.Message;

            var key = requestMessage.Keys.First();
            var value = requestMessage[key] as string;
            string responseValue = false.ToString();
            try
            {
                responseValue = await DMClientBackgroundApplication.Instance.ExecuteDMRequest(key, value);
            } catch (Exception)
            {
                responseValue = false.ToString();
            }

            var response = new ValueSet();
            response.Add("returnValue", responseValue);
            await sender.SendMessageAsync(response);

            deferral.Complete();
        }
        private async void AppServiceConnection_ServiceClosed(AppServiceConnection sender, AppServiceClosedEventArgs args)
        {
            DMClientBackgroundApplication.Instance.Connections.Remove(sender);
            _deferral.Complete();
        }

    }
}
