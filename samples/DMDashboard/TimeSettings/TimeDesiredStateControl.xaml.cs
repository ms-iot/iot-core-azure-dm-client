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
using System.Collections.ObjectModel;
using System.Text;
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
                return "timeInfo";
            }
        }

        private class TimeZoneData
        {
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
                timeZoneData.DaylightName = timeZone.DaylightName;
                timeZoneData.StandardName = timeZone.StandardName;
                timeZoneData.DisplayName = timeZone.DisplayName;
                timeZoneData.Bias = -1 * (int)timeZone.BaseUtcOffset.TotalMinutes;

                TimeZoneInfo.AdjustmentRule[] rules = timeZone.GetAdjustmentRules();
                if (rules.Length != 0)
                {
                    TimeZoneInfo.AdjustmentRule currentRule = rules[rules.Length - 1];
                    // The year is set to 2000 since .Net DateTime object does not accept year 0.
                    // Setting it to 2000 allows the receiving end Newton json library and its users to parse successfully.
                    // The year is not being used by the underlying implementation (see TIME_ZONE_INFORMATION docs).
                    timeZoneData.DaylightDate = "2000-" + currentRule.DaylightTransitionStart.Month.ToString() + "-" + currentRule.DaylightTransitionStart.Week +
                                                "T" + currentRule.DaylightTransitionStart.TimeOfDay.ToString("HH:mm:ssZ");
                    timeZoneData.DaylightBias = -1 * (int)currentRule.DaylightDelta.TotalMinutes;
                    timeZoneData.DaylightDayOfWeek = (int)currentRule.DaylightTransitionStart.DayOfWeek;

                    timeZoneData.StandardDate = "2000-" + currentRule.DaylightTransitionEnd.Month.ToString() + "-" + currentRule.DaylightTransitionEnd.Week +
                                                "T" + currentRule.DaylightTransitionEnd.TimeOfDay.ToString("HH:mm:ssZ");
                    timeZoneData.StandardBias = -1 * 0;
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
            TimeZoneData timeZoneData = (TimeZoneData)DisplayNames.SelectedItem;

            Bias.Text = timeZoneData.Bias.ToString();

            StandardName.Text = timeZoneData.StandardName;
            StandardDate.Text = timeZoneData.StandardDate;
            StandardBias.Text = timeZoneData.StandardBias.ToString();
            StandardDayOfWeek.Text = timeZoneData.StandardDayOfWeek.ToString();

            DaylightName.Text = timeZoneData.DaylightName;
            DaylightDate.Text = timeZoneData.DaylightDate;
            DaylightBias.Text = timeZoneData.DaylightBias.ToString();
            DaylightDayOfWeek.Text = timeZoneData.DaylightDayOfWeek.ToString();
        }

        public string ToJson()
        {
            TimeZoneData timeZoneData = (TimeZoneData)DisplayNames.SelectedItem;
            string timeServer = ((ComboBoxItem)DesiredNtpServer.SelectedItem).Content.ToString();

            StringBuilder sb = new StringBuilder();
            sb.Append("\"timeInfo\" : {\n");
            sb.Append("\"timeZoneDaylightBias\" : " + timeZoneData.DaylightBias + ",\n");
            sb.Append("\"timeZoneDaylightDate\" : \"" + timeZoneData.DaylightDate + "\",\n");
            sb.Append("\"timeZoneDaylightName\" : \"" + timeZoneData.DaylightName + "\",\n");
            sb.Append("\"timeZoneDaylightDayOfWeek\" : " + timeZoneData.DaylightDayOfWeek + ",\n");
            sb.Append("\"timeZoneStandardBias\" : " + timeZoneData.StandardBias + ",\n");
            sb.Append("\"timeZoneStandardDate\" : \"" + timeZoneData.StandardDate + "\",\n");
            sb.Append("\"timeZoneStandardName\" : \"" + timeZoneData.StandardName + "\",\n");
            sb.Append("\"timeZoneStandardDayOfWeek\" : " + timeZoneData.StandardDayOfWeek + ",\n");
            sb.Append("\"timeZoneBias\" : " + timeZoneData.Bias + ",\n");
            sb.Append("\"ntpServer\" : \"" + timeServer + "\"\n");
            sb.Append("}");

            return sb.ToString();
        }

    }
}
