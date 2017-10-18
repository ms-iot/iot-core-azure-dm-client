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

using Microsoft.Devices.Management.DMDataContract;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Windows;
using System.Windows.Controls;

namespace DMDashboard
{
    public partial class TimeDesiredStateControl : UserControl
    {
        public string SectionName
        {
            get
            {
                return TimeSettingsDataContract.SectionName;
            }
        }

        private class TimeZoneData
        {
            public string Id { get; set; }
            public string DisplayName { get; set; }

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

        public TimeDesiredStateControl()
        {
            InitializeComponent();
            PopulateTimeZones();
        }

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

            DisplayNames.ItemsSource = displayNames;
            DisplayNames.SelectedIndex = 0;
        }

        private void OnExpandDetails(object sender, RoutedEventArgs e)
        {
            if (DetailsPane.Visibility == Visibility.Collapsed)
            {
                DetailsPane.Visibility = Visibility.Visible;
            }
            else
            {
                DetailsPane.Visibility = Visibility.Collapsed;
            }
        }

        private void OnTimeZoneChanged(object sender, SelectionChangedEventArgs e)
        {
            DetailsPane.DataContext = (TimeZoneData)DisplayNames.SelectedItem;
        }

        public string ToJson()
        {
            TimeZoneData timeZoneData = (TimeZoneData)DisplayNames.SelectedItem;

            TimeSettingsDataContract.DesiredProperties desiredProperties = new TimeSettingsDataContract.DesiredProperties();

            desiredProperties.ntpServer = ((ComboBoxItem)DesiredNtpServer.SelectedItem).Content.ToString();

            desiredProperties.timeZoneBias = timeZoneData.Bias;

            desiredProperties.timeZoneStandardBias = timeZoneData.StandardBias;
            desiredProperties.timeZoneStandardDate = timeZoneData.StandardDate;
            desiredProperties.timeZoneStandardName = timeZoneData.StandardName;
            desiredProperties.timeZoneStandardDayOfWeek = timeZoneData.StandardDayOfWeek;

            desiredProperties.timeZoneDaylightBias = timeZoneData.DaylightBias;
            desiredProperties.timeZoneDaylightDate = timeZoneData.DaylightDate;
            desiredProperties.timeZoneDaylightName = timeZoneData.DaylightName;
            desiredProperties.timeZoneDaylightDayOfWeek = timeZoneData.DaylightDayOfWeek;

            desiredProperties.timeZoneKeyName = timeZoneData.Id;
            desiredProperties.dynamicDaylightTimeDisabled = false;

            return desiredProperties.ToJsonString();
        }

    }
}
