/*
Copyright 2018 Microsoft
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

using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using Microsoft.Azure.Devices;
using Microsoft.Win32;
using System.Collections.Generic;
using System.IO;
using System.Windows;
using System.Windows.Controls;

namespace DMValidator
{
    public partial class MainWindow : Window
    {
        private const string RegistryStore = "Software\\Microsoft\\DMValidator";
        private const string RegistryConnectionString = "ConnectionString";
        private const string QueryDeviceList = "SELECT deviceId from Devices";
        private const string QueryDeviceId = "DeviceId";

        public MainWindow()
        {
            _initialized = false;

            InitializeComponent();

            LogFilePathBox.Text = Directory.GetCurrentDirectory();

            _logger = new Logger();
            _logger.TargetListView = LogListView;
            _logger.TargetLogPath = LogFilePathBox.Text;

            LoadConnectionString();

            _initialized = true;
        }

        private void LoadConnectionString()
        {
            RegistryKey key = Registry.CurrentUser.OpenSubKey(RegistryStore);
            if (key == null)
            {
                return;
            }
            IoTHubConnectionStringBox.Text = (string)key.GetValue(RegistryConnectionString);
        }

        private void SaveConnectionString()
        {
            if (!_initialized)
            {
                return;
            }

            RegistryKey key = Registry.CurrentUser.CreateSubKey(RegistryStore);
            if (key == null)
            {
                return;
            }

            key.SetValue(RegistryConnectionString, IoTHubConnectionStringBox.Text);
        }

        private async void ListDevicesAsync(ListView targetListControl)
        {
            RegistryManager registryManager = RegistryManager.CreateFromConnectionString(IoTHubConnectionStringBox.Text);
            targetListControl.ItemsSource = null;

            IQuery query = registryManager.CreateQuery(QueryDeviceList);
            IEnumerable<string> results = await query.GetNextAsJsonAsync();
            List<string> deviceIds = new List<string>();

            foreach (string s in results)
            {
                JObject jObject = (JObject)JsonConvert.DeserializeObject(s);
                deviceIds.Add((string)jObject[QueryDeviceId]);
            }

            deviceIds.Sort();
            targetListControl.ItemsSource = deviceIds;
        }

        private void OnListTestDevices(object sender, RoutedEventArgs e)
        {
            ListDevicesAsync(TestDeviceListView);
        }

        /*
        private void OnBrowseScenarioFile(object sender, RoutedEventArgs e)
        {
            Microsoft.Win32.OpenFileDialog dlg = new Microsoft.Win32.OpenFileDialog();
            dlg.DefaultExt = ".json";
            dlg.Filter = "JSon Files (*.json)|*.json";
            Nullable<bool> result = dlg.ShowDialog();
            if (result == true)
            {
                ScenarioFileNameBox.Text = dlg.FileName;
            }
        }
        */
        private async void OnExecuteTestCasesAsync()
        {
            if (TestDeviceListView.SelectedItems.Count != 1)
            {
                MessageBox.Show("Select one device to run the scenarios on.");
                return;
            }

            if (ScenariosListView.SelectedItems.Count == 0)
            {
                MessageBox.Show("Select at least one scenario to run.");
                return;
            }

            _logger.Log(LogLevel.Information, "---- New Run ------------------------------------------------------------------");

            TestParameters testParameters = new TestParameters();
            testParameters.IoTHubConnectionString = IoTHubConnectionStringBox.Text;
            testParameters.IoTHubDeviceId = (string)TestDeviceListView.SelectedItem;

            List<string> summaryList = new List<string>();

            bool allResult = true;
            foreach (var item in ScenariosListView.SelectedItems)
            {
                string scenarioFileName = (string)item;
                _logger.Log(LogLevel.Information, "Processing " + scenarioFileName);

                bool result = await TestScenario.Run(_logger, scenarioFileName, testParameters);

                summaryList.Add((result ? "[ok] " : "[xx] ") + scenarioFileName);
                allResult &= result;
            }

            _logger.Log(LogLevel.Information, "---- Summary ------------------------------------------------------------------");
            foreach (string s in summaryList)
            {
                _logger.Log(LogLevel.Information, s);
            }

            _logger.Log(LogLevel.Information, "---> " + (allResult ? "All succeeded" : "One ore more tests have failed"));
        }

        private void OnExecuteTestCases(object sender, RoutedEventArgs e)
        {
            _logger.CreateNewFile();
            OnExecuteTestCasesAsync();
        }

        private void OnLoadScenarios(object sender, RoutedEventArgs e)
        {
            List<string> fileNames = new List<string>();

            foreach (var f in Directory.EnumerateFiles(@".", "*.json", SearchOption.AllDirectories))
            {
                fileNames.Add(f);
            }

            ScenariosListView.ItemsSource = fileNames;
        }

        private void OnChangeLogFilePath(object sender, RoutedEventArgs e)
        {
            using (var dialog = new System.Windows.Forms.FolderBrowserDialog())
            {
                System.Windows.Forms.DialogResult result = dialog.ShowDialog();
                if (result == System.Windows.Forms.DialogResult.OK)
                {
                    if (LogFilePathBox.Text != dialog.SelectedPath)
                    {
                        LogFilePathBox.Text = dialog.SelectedPath;
                        _logger.TargetLogPath = LogFilePathBox.Text;
                    }
                }
            }
        }

        private void OnOpenLogFilePath(object sender, RoutedEventArgs e)
        {
            System.Diagnostics.Process.Start("C:\\Windows\\explorer.exe", "\"" + LogFilePathBox.Text + "\"");
        }

        private void OnClearLogView(object sender, RoutedEventArgs e)
        {
            LogListView.Items.Clear();
        }

        private void OnConnectionStringChanged(object sender, TextChangedEventArgs e)
        {
            SaveConnectionString();
        }

        Logger _logger;
        bool _initialized;
    }
}
