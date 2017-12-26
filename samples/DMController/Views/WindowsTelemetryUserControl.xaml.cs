using Microsoft.Devices.Management.DMDataContract;
using Newtonsoft.Json.Linq;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;

// The User Control item template is documented at https://go.microsoft.com/fwlink/?LinkId=234236

namespace DMController.Views
{
    public sealed partial class WindowsTelemetryUserControl : UserControl
    {
        public WindowsTelemetryUserControl()
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
        /// Retrieve windows telemetry setting from device twin.
        /// </summary>
        private async void GetReportedButton_ClickAsync(object sender, RoutedEventArgs e)
        {
            var twinResult = await _mainPage.GetTwinData(WindowsTelemetryDataContract.SectionName);
            if (twinResult != null)
            {
                WindowsTelemetryDataContract.ReportedProperties reportedProperties = WindowsTelemetryDataContract.ReportedProperties.FromJsonObject((JObject)twinResult);
                ReportedTextBox.Text = reportedProperties.level;
            }

        }

        /// <summary>
        /// Set windows telemetry setting via device twin.
        /// </summary>
        private async void SetDesiredButton_ClickAsync(object sender, RoutedEventArgs e)
        {
            WindowsTelemetryDataContract.DesiredProperties desiredProperties = new WindowsTelemetryDataContract.DesiredProperties();

            switch (DesiredCombobox.SelectedIndex)
            {
                case 0:
                    desiredProperties.level = WindowsTelemetryDataContract.JsonSecurity;
                    break;
                case 1:
                    desiredProperties.level = WindowsTelemetryDataContract.JsonBasic;
                    break;
                case 2:
                    desiredProperties.level = WindowsTelemetryDataContract.JsonEnhanced;
                    break;
                case 3:
                    desiredProperties.level = WindowsTelemetryDataContract.JsonFull;
                    break;
                default:
                    _mainPage.ShowDialogAsync("Invaid Input", "Desired value not selected! Selected 'security'.");
                    break;
            }

            string refreshingValue = "\"refreshing\"";
            string finalValue = "{" + desiredProperties.ToJsonString() + "}";
            await _mainPage.UpdateTwinData(refreshingValue, finalValue);

        }
    }
}
