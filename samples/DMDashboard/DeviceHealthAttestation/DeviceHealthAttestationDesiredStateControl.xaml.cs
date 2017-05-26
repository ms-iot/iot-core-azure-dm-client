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
using Newtonsoft.Json;
using System.Windows.Controls;

namespace DMDashboard.DeviceHealthAttestation
{
    /// <summary>
    /// Interaction logic for DeviceHealthAttestationDesiredStateControl.xaml
    /// </summary>
    public partial class DeviceHealthAttestationDesiredStateControl : UserControl
    {
        public DeviceHealthAttestationDesiredStateControl()
        {
            InitializeComponent();
            this.ServerEndpoint.Text = "";
            this.ReportInterval.Text = "0";
        }

        public string ToJson()
        {
            return "\"" + DeviceHealthAttestationDataContract.JsonSectionName + "\" : " + JsonConvert.SerializeObject(UIToValue());
        }

        private DeviceHealthAttestationDataContract.DesiredProperty UIToValue()
        {
            return new DeviceHealthAttestationDataContract.DesiredProperty()
            {
                Endpoint = this.ServerEndpoint.Text,
                ReportIntervalInSeconds = int.Parse(this.ReportInterval.Text)
            };
        }
    }
}
