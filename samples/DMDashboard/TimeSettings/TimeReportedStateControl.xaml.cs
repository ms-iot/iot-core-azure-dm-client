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
using Newtonsoft.Json.Linq;
using System.Windows.Controls;

namespace DMDashboard
{
    public partial class TimeReportedStateControl : UserControl
    {
        public TimeReportedStateControl()
        {
            InitializeComponent();
        }

        public void FromJson(JObject jsonObject)
        {
            TimeSettingsDataContract.ReportedProperties properties = TimeSettingsDataContract.ReportedProperties.FromJsonObject(jsonObject);
            LocalTime.Text = properties.localTime;
            NtpServer.Text = properties.ntpServer;
            DynamicDaylightTimeDisabled.Text = properties.dynamicDaylightTimeDisabled ? "True" : "False";
            ZoneKeyName.Text = properties.timeZoneKeyName;
            Bias.Text = properties.timeZoneBias.ToString();
            StandardName.Text = properties.timeZoneStandardName;
            StandardDate.Text = properties.timeZoneStandardDate;
            StandardBias.Text = properties.timeZoneStandardBias.ToString();
            StandardDayOfWeek.Text = properties.timeZoneStandardDayOfWeek.ToString();
            DaylightName.Text = properties.timeZoneDaylightName;
            DaylightDate.Text = properties.timeZoneDaylightDate;
            DaylightBias.Text = properties.timeZoneDaylightBias.ToString();
            DaylightDayOfWeek.Text = properties.timeZoneDaylightDayOfWeek.ToString();
        }
    }
}
