using Microsoft.Devices.Management.DMDataContract;
using Newtonsoft.Json.Linq;
using System;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using System.Timers;

// The User Control item template is documented at https://go.microsoft.com/fwlink/?LinkId=234236

namespace DMController.Views
{
    public sealed partial class DeviceInfoUserControl : UserControl
    {
        public DeviceInfoUserControl()
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
        /// Public function to set the current device and display the device info. 
        /// </summary>
        /// <param name="deviceSelectedName">The device name</param>
        public void SetCurrentDevice(string deviceSelectedName)
        {
            ClearDeviceInfoUI();
            DeviceInfoTitle.Text = deviceSelectedName;
            UpdateDeviceInfoUIAsync();
        }

        /// <summary>
        /// Retrieve device info from device twin.
        /// </summary>
        private async void UpdateDeviceInfoUIAsync()
        {
            var deviceInfoResult = await _mainPage.GetTwinData(DeviceInfoDataContract.SectionName);
            if (deviceInfoResult != null)
            {
                DeviceInfoDataContract.ReportedProperties reportedProperties = DeviceInfoDataContract.ReportedProperties.FromJsonObject((JObject)deviceInfoResult);
                DeviceId.Text = reportedProperties.id;
                DeviceManufactuer.Text = reportedProperties.manufacturer;
                DeviceModel.Text = reportedProperties.model;
                DeviceDmVer.Text = reportedProperties.dmVer;
                DeviceLang.Text = reportedProperties.lang;
                DeviceType.Text = reportedProperties.type;
                DeviceOEM.Text = reportedProperties.oem;
                DeviceHwVer.Text = reportedProperties.hwVer;
                DeviceSwVer.Text = reportedProperties.fwVer;
                DeviceOsVer.Text = reportedProperties.osVer;
                DevicePlatform.Text = reportedProperties.platform;
                DeviceProcessorType.Text = reportedProperties.processorType;
                DeviceRadioSwVer.Text = reportedProperties.radioSwVer;
                DeviceDisplayResolution.Text = reportedProperties.displayResolution;
                DeviceCommercializationOpe.Text = reportedProperties.commercializationOperator;
                DeviceProcessorArch.Text = reportedProperties.processorArchitecture;
                DeviceName.Text = reportedProperties.name;
                DeviceStorage.Text = reportedProperties.totalStorage;
                DeviceMemory.Text = reportedProperties.totalMemory;
                DeviceSecureBoot.Text = reportedProperties.secureBootState;
                DeviceOsEdition.Text = reportedProperties.osEdition;
                DeviceBatteryStatus.Text = reportedProperties.batteryStatus;
                DeviceBatteryRemain.Text = reportedProperties.batteryRemaining;
                DeviceBatteryRuntime.Text = reportedProperties.batteryRuntime;
            }
        }

        /// <summary>
        /// Request new device info via device twin.
        /// The device will request new device info; wait 5 seconds and update the UI with new device info data. 
        /// </summary>
        private async void DeviceInfoUpdateButton_ClickAsync(object sender, RoutedEventArgs e)
        {
            string refreshingValue = "\"refreshing\"";
            string finalValue = "{ \"" + DeviceInfoDataContract.SectionName + "\" : {}}";
            await _mainPage.UpdateTwinData(refreshingValue, finalValue);

            // Create a timer and set a five second interval.
            var aTimer = new System.Timers.Timer();
            aTimer.Interval = 5000;
            // Update the UI with new device info
            aTimer.Elapsed += OnTimedEventAsync;
            // Start the timer
            aTimer.Enabled = true;
        }

        /// <summary>
        /// Force the device to report all its properties via direct method.
        /// </summary>
        private void DeviceReportAllButton_Click(object sender, RoutedEventArgs e)
        {
            var result = _mainPage.CallDeviceMethod(CommonDataContract.ReportAllAsync, "{}");
            // Create a timer and set a five second interval.
            var aTimer = new System.Timers.Timer();
            aTimer.Interval = 60000;
            // Hook up the Elapsed event for the timer. 
            aTimer.Elapsed += OnTimedEventAsync;
            // Start the timer
            aTimer.Enabled = true;
        }

        /// <summary>
        /// Timer callback event to update UI.
        /// </summary>
        private async void OnTimedEventAsync(object sender, ElapsedEventArgs e)
        {
            await Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, () =>
            {
                //UI code here
                UpdateDeviceInfoUIAsync();

            });
        }

        /// <summary>
        /// Clear the fields on UI. 
        /// </summary>
        private void ClearDeviceInfoUI()
        {
            DeviceId.Text = "-";
            DeviceManufactuer.Text = "-";
            DeviceModel.Text = "-";
            DeviceDmVer.Text = "-";
            DeviceLang.Text = "-";
            DeviceType.Text = "-";
            DeviceOEM.Text = "-";
            DeviceHwVer.Text = "-";
            DeviceSwVer.Text = "-";
            DeviceOsVer.Text = "-";
            DevicePlatform.Text = "-";
            DeviceProcessorType.Text = "-";
            DeviceRadioSwVer.Text = "-";
            DeviceDisplayResolution.Text = "-";
            DeviceCommercializationOpe.Text = "-";
            DeviceProcessorArch.Text = "-";
            DeviceName.Text = "-";
            DeviceStorage.Text = "-";
            DeviceMemory.Text = "-";
            DeviceSecureBoot.Text = "-";
            DeviceOsEdition.Text = "-";
            DeviceBatteryStatus.Text = "-";
            DeviceBatteryRemain.Text = "-";
            DeviceBatteryRuntime.Text = "-";
        }


    }
}
