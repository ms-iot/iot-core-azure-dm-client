using System;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Microsoft.Devices.Management.DMDataContract;
using Newtonsoft.Json.Linq;

// The User Control item template is documented at https://go.microsoft.com/fwlink/?LinkId=234236

namespace DMController.Views
{
    public sealed partial class RebootUserControl : UserControl
    {
        public RebootUserControl()
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
        /// Initiate reboot on device via direct method.
        /// </summary>
        private void RebootButton_Click(object sender, RoutedEventArgs e)
        {
            var result = _mainPage.CallDeviceMethod(RebootCmdDataContract.RebootCmdAsync, "{}");
        }

        /// <summary>
        /// Retrieve reboot info from device twin.
        /// </summary>
        private async void GetRebootInfoButton_ClickAsync(object sender, RoutedEventArgs e)
        {
            var twinResult = await _mainPage.GetTwinData(RebootInfoDataContract.SectionName);
            if (twinResult != null)
            {
                RebootInfoDataContract.ReportedProperties reportedProperties = RebootInfoDataContract.ReportedProperties.FromJsonObject((JObject)twinResult);
                RebootInfoLastRebootTime.Text = reportedProperties.lastBootTime;
                RebootInfoSingleReboot.Text = reportedProperties.singleRebootTime;
                RebootInfoDailyReboot.Text = reportedProperties.dailyRebootTime;
            }
        }

        /// <summary>
        /// Retrieve the outcome of executing the reboot command from device twin.
        /// </summary>
        private async void GetRebootResponseButton_ClickAsync(object sender, RoutedEventArgs e)
        {
            var twinResult = await _mainPage.GetTwinData(RebootCmdDataContract.SectionName);
            if (twinResult != null)
            {
                RebootCmdDataContract.ReportedProperties reportedProperties = RebootCmdDataContract.ReportedProperties.FromJsonObject((JObject)twinResult);
                ReportedResponseText.Text = reportedProperties.response;
            }
        }

        /// <summary>
        /// Enable or disable reboot setting input. 
        /// </summary>
        private void RebootInfoSingleChkbx_Click(object sender, RoutedEventArgs e)
        {
            if (RebootInfoSingleChkbx.IsChecked == true)
            {
                RebootInfoSingleRebootDateInput.IsEnabled = true;
                RebootInfoSingleRebootTimeInput.IsEnabled = true;
            }
            else
            {
                RebootInfoSingleRebootDateInput.IsEnabled = false;
                RebootInfoSingleRebootTimeInput.IsEnabled = false;
            }
        }

        /// <summary>
        /// Enable or disable reboot setting input. 
        /// </summary>
        private void RebootInfoDailyChkbx_Click(object sender, RoutedEventArgs e)
        {
            if (RebootInfoDailyChkbx.IsChecked == true)
            {
                RebootInfoDailyRebootDateInput.IsEnabled = true;
                RebootInfoDailyRebootTimeInput.IsEnabled = true;

            }
            else
            {
                RebootInfoDailyRebootDateInput.IsEnabled = false;
                RebootInfoDailyRebootTimeInput.IsEnabled = false;
            }
        }

        /// <summary>
        /// Set schedule reboot info via device twin.
        /// </summary>
        private async void SetScheduleRebootButton_ClickAsync(object sender, RoutedEventArgs e)
        {
            string dailyReboot = string.Empty;
            string singleReboot = string.Empty;

            if (RebootInfoDailyChkbx.IsChecked == true)
            {
                var pickedRebootDate = RebootInfoDailyRebootDateInput.Date;
                var pickedRebootTime = RebootInfoDailyRebootTimeInput.Time;
                DateTime resultDailyDateTime = new DateTime(pickedRebootDate.Year, pickedRebootDate.Month, pickedRebootDate.Day, pickedRebootTime.Hours, pickedRebootTime.Minutes, 0, DateTimeKind.Utc);
                dailyReboot = (resultDailyDateTime.ToString("yyyy-MM-ddTHH:mm:ssZ"));
            }
            if (RebootInfoSingleChkbx.IsChecked == true)
            {
                var pickedRebootDate = RebootInfoSingleRebootDateInput.Date;
                var pickedRebootTime = RebootInfoSingleRebootTimeInput.Time;
                DateTime resulSingleDateTime = new DateTime(pickedRebootDate.Year, pickedRebootDate.Month, pickedRebootDate.Day, pickedRebootTime.Hours, pickedRebootTime.Minutes, 0, DateTimeKind.Utc);
                singleReboot = (resulSingleDateTime.ToString("yyyy-MM-ddTHH:mm:ssZ"));
            }

            RebootInfoDataContract.DesiredProperties desiredProperties = new RebootInfoDataContract.DesiredProperties();
            desiredProperties.singleRebootTime = singleReboot;
            desiredProperties.dailyRebootTime = dailyReboot;

            string refreshingValue = "\"refreshing\"";
            string finalValue = "{" + desiredProperties.ToJsonString() + "}";
            await _mainPage.UpdateTwinData(refreshingValue, finalValue);
        }
    }
}
