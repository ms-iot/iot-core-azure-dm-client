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

        public void FromJson(JObject json)
        {
            string notFound = "<not found>";
            LocalTime.Text = Utils.GetString(json, "localTime", notFound);
            NtpServer.Text = Utils.GetString(json, "ntpServer", notFound);
            Bias.Text = Utils.GetString(json, "timeZoneBias", notFound);
            StandardName.Text = Utils.GetString(json, "timeZoneStandardName", notFound);
            StandardDate.Text = Utils.GetString(json, "timeZoneStandardDate", notFound);
            StandardBias.Text = Utils.GetString(json, "timeZoneStandardBias", notFound);
            StandardDayOfWeek.Text = Utils.GetString(json, "timeZoneStandardDayOfWeek", notFound);
            DaylightName.Text = Utils.GetString(json, "timeZoneDaylightName", notFound);
            DaylightDate.Text = Utils.GetString(json, "timeZoneDaylightDate", notFound);
            DaylightBias.Text = Utils.GetString(json, "timeZoneDaylightBias", notFound);
            DaylightDayOfWeek.Text = Utils.GetString(json, "timeZoneDaylightDayOfWeek", notFound);
        }
    }
}
