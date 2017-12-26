using Microsoft.Devices.Management.DMDataContract;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;

// The User Control item template is documented at https://go.microsoft.com/fwlink/?LinkId=234236

namespace DMController.Views
{
    public sealed partial class TimeUserControl : UserControl
    {
        public TimeUserControl()
        {
            this.InitializeComponent();
            PopulateTimeZones();
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
        /// Populate the timezone info and update on UI.
        /// </summary>
        private void PopulateTimeZones()
        {
            ReadOnlyCollection<TimeZoneInfo> tzCollection = TimeZoneInfo.GetSystemTimeZones();

            List<TimeZoneData> displayNames = new List<TimeZoneData>();

            foreach (TimeZoneInfo timeZone in tzCollection)
            {
                TimeZoneData timeZoneData = new TimeZoneData();
                timeZoneData.Id = timeZone.Id;
                timeZoneData.DaylightName = timeZone.DaylightName;
                timeZoneData.StandardName = timeZone.StandardName;
                timeZoneData.DisplayName = timeZone.DisplayName;
                timeZoneData.Bias = -1 * (int)timeZone.BaseUtcOffset.TotalMinutes;

                TimeZoneInfo.AdjustmentRule[] rules = timeZone.GetAdjustmentRules();
                if (rules.Length != 0)
                {
                    TimeZoneInfo.AdjustmentRule currentRule = rules[rules.Length - 1];

                    int daylightYear = 0;
                    if (currentRule.DaylightTransitionStart.TimeOfDay.Year != 1)    // .net uses 1 so that DateTime accepts it.
                    {
                        daylightYear = currentRule.DaylightTransitionStart.TimeOfDay.Year;
                    }

                    timeZoneData.DaylightDate = daylightYear +
                                                "-" + currentRule.DaylightTransitionStart.Month.ToString() +
                                                "-" + currentRule.DaylightTransitionStart.Week +
                                                "T" + currentRule.DaylightTransitionStart.TimeOfDay.ToString("HH:mm:ss");

                    timeZoneData.DaylightBias = -1 * (int)currentRule.DaylightDelta.TotalMinutes;
                    timeZoneData.DaylightDayOfWeek = (int)currentRule.DaylightTransitionStart.DayOfWeek;

                    int standardYear = 0;
                    if (currentRule.DaylightTransitionEnd.TimeOfDay.Year != 1)    // .net uses 1 so that DateTime accepts it.
                    {
                        standardYear = currentRule.DaylightTransitionEnd.TimeOfDay.Year;
                    }

                    timeZoneData.StandardDate = standardYear +
                                                "-" + currentRule.DaylightTransitionEnd.Month.ToString() +
                                                "-" + currentRule.DaylightTransitionEnd.Week +
                                                "T" + currentRule.DaylightTransitionEnd.TimeOfDay.ToString("HH:mm:ss");
                    timeZoneData.StandardBias = 0;
                    timeZoneData.StandardDayOfWeek = (int)currentRule.DaylightTransitionEnd.DayOfWeek;
                }

                displayNames.Add(timeZoneData);
            }

            TimeZoneCombobox.ItemsSource = displayNames;
            TimeZoneCombobox.SelectedIndex = 0;
        }

        /// <summary>
        /// Private class to describe Time Zone Data. 
        /// </summary>
        private class TimeZoneData
        {
            public string Id { get; set; }
            public string DisplayName { get; set; }
            public bool DisableDynamicDaylightTime { get; set; } = false;
            public int Bias { get; set; }

            public string StandardDate { get; set; }
            public int StandardBias { get; set; }
            public string StandardName { get; set; }
            public int StandardDayOfWeek { get; set; }

            public string DaylightDate { get; set; }
            public int DaylightBias { get; set; }
            public string DaylightName { get; set; }
            public int DaylightDayOfWeek { get; set; }
        }

        /// <summary>
        /// Retrieve Time Zone and Synchronization Server from device twin.
        /// </summary>
        private async void GetTimeInfoButton_ClickAsync(object sender, RoutedEventArgs e)
        {
            var twinResult = await _mainPage.GetTwinData(TimeSettingsDataContract.SectionName);
            if (twinResult != null)
            {
                TimeSettingsDataContract.ReportedProperties reportedProperties = TimeSettingsDataContract.ReportedProperties.FromJsonObject((JObject)twinResult);
                NtpServer.Text = reportedProperties.ntpServer;
                LocalTime.Text = reportedProperties.localTime;
                TimezoneKeyName.Text = reportedProperties.timeZoneKeyName;
                DynamicDaylightTime.Text = reportedProperties.dynamicDaylightTimeDisabled.ToString();
                TimeZoneBias.Text = reportedProperties.timeZoneBias.ToString();
                TimeZoneStandardName.Text = reportedProperties.timeZoneStandardName;
                TimeZoneStandardDate.Text = reportedProperties.timeZoneStandardDate;
                TimeZoneStandardBias.Text = reportedProperties.timeZoneStandardBias.ToString();
                TimeZoneStandardDayOfWeek.Text = reportedProperties.timeZoneStandardDayOfWeek.ToString();
                TimeZoneDaylightName.Text = reportedProperties.timeZoneDaylightName;
                TimeZoneDaylightDate.Text = reportedProperties.timeZoneDaylightDate;
                TimeZoneDaylightBias.Text = reportedProperties.timeZoneDaylightBias.ToString();
                TimeZoneDaylightDayOfWeek.Text = reportedProperties.timeZoneDaylightDayOfWeek.ToString();
            }
        }

        /// <summary>
        /// Time zone dropdown input changed, update the UI.
        /// </summary>
        private void TimeZoneInput_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            ComboBox comboBox = (ComboBox)sender;
            if (comboBox != null)
            {
                TimeZoneData selectedTimeZoneData = (TimeZoneData)comboBox.SelectedItem;

                TimezoneKeyNameInput.Text = selectedTimeZoneData.DisplayName;
                DynamicDaylightTimeInput.Text = selectedTimeZoneData.DisableDynamicDaylightTime.ToString();

                TimeZoneBiasInput.Text = selectedTimeZoneData.Bias.ToString();
                TimeZoneStandardNameInput.Text = selectedTimeZoneData.StandardName ?? string.Empty;
                TimeZoneStandardDateInput.Text = selectedTimeZoneData.StandardDate ?? string.Empty;
                TimeZoneStandardBiasInput.Text = selectedTimeZoneData.StandardBias.ToString() ?? string.Empty;
                TimeZoneStandardDayOfWeekInput.Text = selectedTimeZoneData.StandardDayOfWeek.ToString() ?? string.Empty;
                TimeZoneDaylightNameInput.Text = selectedTimeZoneData.DaylightName ?? string.Empty;
                TimeZoneDaylightDateInput.Text = selectedTimeZoneData.DaylightDate ?? string.Empty;
                TimeZoneDaylightBiasInput.Text = selectedTimeZoneData.DaylightBias.ToString() ?? string.Empty;
                TimeZoneDaylightDayOfWeekInput.Text = selectedTimeZoneData.StandardDayOfWeek.ToString() ?? string.Empty;
            }
        }

        /// <summary>
        /// Set device Time Zone and Synchronization Server via device twin.
        /// </summary>
        private async void SetTimeInfoButton_ClickAsync(object sender, RoutedEventArgs e)
        {
            TimeZoneData selectedTimeZoneData = (TimeZoneData)TimeZoneCombobox.SelectedItem;
            if (selectedTimeZoneData == null)
            {
                _mainPage.ShowDialogAsync("Invaid Input", "Time Zone value is not selected");
                return;
            }

            TimeSettingsDataContract.DesiredProperties desiredProperties = new TimeSettingsDataContract.DesiredProperties();
            desiredProperties.ntpServer = NtpServerCombobox.SelectedValue.ToString();
            desiredProperties.timeZoneBias = selectedTimeZoneData.Bias;

            desiredProperties.timeZoneStandardBias = selectedTimeZoneData.StandardBias;
            desiredProperties.timeZoneStandardDate = selectedTimeZoneData.StandardDate;
            desiredProperties.timeZoneStandardName = selectedTimeZoneData.StandardName;
            desiredProperties.timeZoneStandardDayOfWeek = selectedTimeZoneData.StandardDayOfWeek;

            desiredProperties.timeZoneDaylightBias = selectedTimeZoneData.DaylightBias;
            desiredProperties.timeZoneDaylightDate = selectedTimeZoneData.DaylightDate;
            desiredProperties.timeZoneDaylightName = selectedTimeZoneData.DaylightName;
            desiredProperties.timeZoneDaylightDayOfWeek = selectedTimeZoneData.DaylightDayOfWeek;

            desiredProperties.timeZoneKeyName = selectedTimeZoneData.Id;
            desiredProperties.dynamicDaylightTimeDisabled = selectedTimeZoneData.DisableDynamicDaylightTime;

            string refreshingValue = "\"refreshing\"";
            string finalValue = "{" + desiredProperties.ToJsonString() + "}";
            await _mainPage.UpdateTwinData(refreshingValue, finalValue);
        }

        /// <summary>
        /// Retrieve Time Service setting from device twin.
        /// </summary>
        private async void GetTimeSettingButton_ClickAsync(object sender, RoutedEventArgs e)
        {
            var twinResult = await _mainPage.GetTwinData(TimeServiceDataContract.SectionName);
            if (twinResult != null)
            {
                TimeServiceDataContract.ReportedProperties reportedProperties = TimeServiceDataContract.ReportedProperties.FromJsonObject((JObject)twinResult);
                Enabled.Text = reportedProperties.enabled;
                Startup.Text = reportedProperties.startup;
                Started.Text = reportedProperties.started;
                SourcePriority.Text = reportedProperties.sourcePriority;
            }
        }

        /// <summary>
        /// Set Time Service setting from device twin.
        /// </summary>
        private async void SetTimeSettingButton_ClickAsync(object sender, RoutedEventArgs e)
        {
            TimeServiceDataContract.DesiredProperties desiredProperties = new TimeServiceDataContract.DesiredProperties();
            desiredProperties.enabled = EnabledCombobox.SelectedValue.ToString();
            desiredProperties.startup = StartupCombobox.SelectedValue.ToString();
            desiredProperties.started = StartedCombobox.SelectedValue.ToString();
            desiredProperties.sourcePriority = SourcePriorityCombobox.SelectedValue.ToString();

            string refreshingValue = "\"refreshing\"";
            string finalValue = "{" + desiredProperties.ToJsonString() + "}";
            await _mainPage.UpdateTwinData(refreshingValue, finalValue);
        }
    }
}
