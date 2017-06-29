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
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;

namespace DMDashboard.StorageManagement
{
    public partial class DeviceDeleteFile : Window
    {
        const string PropFolder = "folder";
        const string PropFile = "file";
        const string DeleteDMFileMethod = "windows.deleteDMFile";

        public DeviceDeleteFile(DeviceTwinAndMethod azureDevice)
        {
            InitializeComponent();

            _azureDevice = azureDevice;
            FileSelectionControl.AzureDevice =  _azureDevice;
        }

        private async Task DeleteAsync()
        {
            string fileName = FileSelectionControl.SelectedFileName;
            if (String.IsNullOrEmpty(fileName))
            {
                return;
            }

            string folderName = FileSelectionControl.SelectedFolderName;
            if (String.IsNullOrEmpty(folderName))
            {
                return;
            }

            StringBuilder parameters = new StringBuilder();
            parameters.Append("{\n");
            parameters.Append("  \"" + PropFolder + "\" : \"" + folderName + "\",\n");
            parameters.Append("  \"" + PropFile + "\" : \"" + fileName + "\"\n");
            parameters.Append("}");

            CancellationToken cancellationToken = new CancellationToken();
            DeviceMethodReturnValue result = await _azureDevice.CallDeviceMethod(DeleteDMFileMethod, parameters.ToString(), new TimeSpan(0, 0, 30), cancellationToken);

            MessageBox.Show("Triggered file delete...");
            FileSelectionControl.EnumFilesAsync();
        }

        private void OnDelete(object sender, RoutedEventArgs e)
        {
            DeleteAsync();
        }

        private void OnClose(object sender, RoutedEventArgs e)
        {
            Close();
        }

        DeviceTwinAndMethod _azureDevice;
    }
}
