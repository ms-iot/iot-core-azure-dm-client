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

        public void FromJson(JObject json)
        {
            LocalTime.Text = (string)json.Property("localTime").Value;
            NtpServer.Text = (string)json.Property("ntpServer").Value;
            Bias.Text = (string)json.Property("timeZoneBias").Value;
            StandardName.Text = (string)json.Property("timeZoneStandardName").Value;
            StandardDate.Text = (string)json.Property("timeZoneStandardDate").Value;
            StandardBias.Text = (string)json.Property("timeZoneStandardBias").Value;
            StandardDayOfWeek.Text = (string)json.Property("timeZoneStandardDayOfWeek").Value;
            DaylightName.Text = (string)json.Property("timeZoneDaylightName").Value;
            DaylightDate.Text = (string)json.Property("timeZoneDaylightDate").Value;
            DaylightBias.Text = (string)json.Property("timeZoneDaylightBias").Value;
            DaylightDayOfWeek.Text = (string)json.Property("timeZoneDaylightDayOfWeek").Value;
        }
    }
}
