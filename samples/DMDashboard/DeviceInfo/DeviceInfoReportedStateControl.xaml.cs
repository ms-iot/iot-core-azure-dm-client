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
    public partial class DeviceInfoReportedStateControl : UserControl
    {
        public string SectionName
        {
            get
            {
                return DeviceInfoDataContract.SectionName;
            }
        }

        public DeviceInfoReportedStateControl()
        {
            InitializeComponent();
        }

        public void FromJsonObject(JObject json)
        {
            DeviceInfoDataContract.ReportedProperties reportedProperties = DeviceInfoDataContract.ReportedProperties.FromJsonObject(json);

            Id.Text = reportedProperties.id;
            Manufacturer.Text = reportedProperties.manufacturer;
            Model.Text = reportedProperties.model;
            DmVer.Text = reportedProperties.dmVer;

            Lang.Text = reportedProperties.lang;
            Type.Text = reportedProperties.type;
            OEM.Text = reportedProperties.oem;
            HwVer.Text = reportedProperties.hwVer;
            FwVer.Text = reportedProperties.fwVer;

            OSVer.Text = reportedProperties.osVer;
            Platform.Text = reportedProperties.platform;
            ProcessorType.Text = reportedProperties.processorType;
            RadioSwVer.Text = reportedProperties.radioSwVer;
            DisplayResolution.Text = reportedProperties.displayResolution;

            CommercializationOperator.Text = reportedProperties.commercializationOperator;
            ProcessorArchitecture.Text = reportedProperties.processorArchitecture;
            DeviceName.Text = reportedProperties.name;
            TotalStorage.Text = reportedProperties.totalStorage;
            TotalMemory.Text = reportedProperties.totalMemory;

            SecureBootState.Text = reportedProperties.secureBootState;
            OSEdition.Text = reportedProperties.osEdition;
            BatteryStatus.Text = reportedProperties.batteryStatus;
            BatteryRemaining.Text = reportedProperties.batteryRemaining;
            BatteryRuntime.Text = reportedProperties.batteryRuntime;
        }
    }
}
