using Microsoft.Devices.Management.DMDataContract;
using Newtonsoft.Json;
using System;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;

// The User Control item template is documented at https://go.microsoft.com/fwlink/?LinkId=234236

namespace DMController.Views
{
    public sealed partial class DHAUserControl : UserControl
    {
        public DHAUserControl()
        {
            this.InitializeComponent();
        }

        private MainPage _mainPage;
        /// <summary>
        /// Saves the reference to MainPage object
        /// </summary>
        /// <param name="mainPage">The MainPage object.</param>
        public void SetMainPage(MainPage mainPage)
        {
            _mainPage = mainPage;
        }

        /// <summary>
        /// Retrieve Health Attestation Status from device twin.
        /// </summary>
        private async void GetDHAStatusButton_ClickAsync(object sender, RoutedEventArgs e)
        {
            var twinResult = await _mainPage.GetTwinData(DeviceHealthAttestationDataContract.JsonSectionName);
            if (twinResult != null)
            {
                var reportedProperties = twinResult.ToObject<DeviceHealthAttestationDataContract.ReportedProperty>();
                if (reportedProperties.status != null)
                    DHAstatus.Text = reportedProperties.status;
            }
        }

        /// <summary>
        /// Update Health Attestation Status via device twin.
        /// </summary>
        private async void SetDHAButton_ClickAsync(object sender, RoutedEventArgs e)
        {
            DeviceHealthAttestationDataContract.DesiredProperty desiredProperties = new DeviceHealthAttestationDataContract.DesiredProperty();
            int intervalInt = -1;
            if (Int32.TryParse(ReportIntervalInSecondsInput.Text, out intervalInt))
            {
                desiredProperties.ReportIntervalInSeconds = intervalInt;
                desiredProperties.Endpoint = EnpointInput.Text;

                string refreshingValue = "\"refreshing\"";
                string finalValue = "{\"" + DeviceHealthAttestationDataContract.JsonSectionName + "\" : " + JsonConvert.SerializeObject(desiredProperties) + "}";
                await _mainPage.UpdateTwinData(refreshingValue, finalValue);
            }
            else
            {
                _mainPage.ShowDialogAsync("Invaid Input", "Please enter an Integer value for Report Interval");
                return;
            }

        }

    }
}
