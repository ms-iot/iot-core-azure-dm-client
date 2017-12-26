using Microsoft.Devices.Management.DMDataContract;
using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Newtonsoft.Json.Linq;

// The User Control item template is documented at https://go.microsoft.com/fwlink/?LinkId=234236

namespace DMController.Views
{
    public sealed partial class DiagnosticLogsUserControl : UserControl
    {
        public DiagnosticLogsUserControl()
        {
            this.InitializeComponent();
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
        /// Retrieve providers and collectors from device twin.
        /// </summary>
        private async void ListCollectorsButton_ClickAsync(object sender, RoutedEventArgs e)
        {
            var twinResult = await _mainPage.GetTwinData(EventTracingDataContract.SectionName);
            if (twinResult != null) EventTracingJsonToUI(twinResult);
        }

        /// <summary>
        /// A collector is selected, update textboxes with selected collector. 
        /// </summary>
        private void ListViewCollectors_ItemClick(object sender, ItemClickEventArgs e)
        {
            CollectorReported collectorReported = (CollectorReported)e.ClickedItem;

            CollectorNameInput.Text = collectorReported.Name;
            CollectorModeCombobox.SelectedIndex = (int)collectorReported.CollectorInners.traceMode;
            CollectorMaxSizeInput.Text = collectorReported.CollectorInners.logFileSizeLimitMB.ToString();
            CollectorFolderNameInput.Text = collectorReported.CollectorInners.logFileFolder;
            CollectorFileNameInput.Text = collectorReported.CollectorInners.logFileName;
            CollectorStartedChckbx.IsChecked = collectorReported.CollectorInners.started;

            if (collectorReported.CollectorInners.providers.Count > 0)
            {
                var provider = collectorReported.CollectorInners.providers[0];
                AddProviderChckbx_Checked(sender, e);
                ProviderGUIDInput.Text = provider.guid;
                ProviderTraceLevelCombobox.SelectedIndex = (int)provider.traceLevel;
                ProviderKeywordsInput.Text = provider.keywords;
                ProviderEnabledChckbx.IsChecked = provider.enabled;
            }
            else
            {
                AddProviderChckbx_Unchecked(sender, e);
            }
        }

        /// <summary>
        /// Request device to report all collectors via device twin.
        /// </summary>
        private async void GetCollectorsButton_ClickAync(object sender, RoutedEventArgs e)
        {
            string refreshingValue = "\"refreshing\"";
            string finalValue = "{ \"eventTracingCollectors\" : { \"?\": \"" + CollectorCombobox.SelectedValue + "\" } }";
            await _mainPage.UpdateTwinData(refreshingValue, finalValue);
        }

        /// <summary>
        /// Add / Edit a collector via device twin.
        /// </summary>
        private async void SetCollectorButton_ClickAsync(object sender, RoutedEventArgs e)
        {
            string collectorName = CollectorNameInput.Text;
            string collectorMaxSize = CollectorMaxSizeInput.Text;
            if (collectorName.Length == 0 || collectorMaxSize.Length == 0)
            {
                _mainPage.ShowDialogAsync("Invaid Input", "Please make sure Collector Name and Max Size is not empty");
                return;
            }
            int collectorMaxFileSize = 0;
            if (!Int32.TryParse(CollectorMaxSizeInput.Text, out collectorMaxFileSize))
            {
                _mainPage.ShowDialogAsync("Invaid Input", " Max Size is not Integer");
                return;
            }

            EventTracingDataContract.CollectorInner collectorInner = new EventTracingDataContract.CollectorInner
            {
                name = collectorName,
                traceMode = EventTracingDataContract.TraceModeFromJsonString(CollectorModeCombobox.SelectedValue.ToString()),
                logFileSizeLimitMB = collectorMaxFileSize,
                logFileFolder = CollectorFolderNameInput.Text,
                logFileName = CollectorFileNameInput.Text,
                started = CollectorStartedChckbx.IsChecked == true
            };

            if (AddProviderChckbx.IsChecked == true)
            {
                if (ProviderGUIDInput.Text.Length == 0)
                {
                    _mainPage.ShowDialogAsync("Invaid Input", "Please make sure Provider GUID is not empty");
                    return;
                }
                EventTracingDataContract.Provider provider = new EventTracingDataContract.Provider();
                provider.guid = ProviderGUIDInput.Text;
                provider.traceLevel = EventTracingDataContract.TraceLevelFromJsonString(ProviderTraceLevelCombobox.SelectedValue.ToString());
                provider.keywords = ProviderKeywordsInput.Text;
                provider.enabled = ProviderEnabledChckbx.IsChecked == true;
                collectorInner.providers.Add(provider);
            }

            string refreshingValue = "\"refreshing\"";
            string finalValue = "{ \"eventTracingCollectors\" : {\"" + collectorName + "\": {\"applyProperties\" : " + JsonConvert.SerializeObject(collectorInner.ToJsonObject()) + ", \"reportProperties\": \"yes\"}}}";
            await _mainPage.UpdateTwinData(refreshingValue, finalValue);
        }

        /// <summary>
        /// Uncheck add provider option. 
        /// </summary>
        private void AddProviderChckbx_Unchecked(object sender, RoutedEventArgs e)
        {
            ProviderGUIDInput.IsEnabled = false;
            ProviderTraceLevelCombobox.IsEnabled = false;
            ProviderKeywordsInput.IsEnabled = false;
            ProviderEnabledChckbx.IsEnabled = false;
            //ProviderTypeInput.IsEnabled = false;
        }

        /// <summary>
        /// Check add provider option. 
        /// </summary>
        private void AddProviderChckbx_Checked(object sender, RoutedEventArgs e)
        {
            ProviderGUIDInput.IsEnabled = true;
            ProviderTraceLevelCombobox.IsEnabled = true;
            ProviderKeywordsInput.IsEnabled = true;
            ProviderEnabledChckbx.IsEnabled = true;
            //ProviderTypeInput.IsEnabled = true;
        }

        /// <summary>
        /// Get a list of IoTDM subfolder names via direct method.
        /// </summary>
        private async void GetFoldersButton_ClickAsync(object sender, RoutedEventArgs e)
        {
            string result = await _mainPage.CallDeviceMethod("windows.enumDMFolders", "{}");
            try
            {
                JObject json = JObject.Parse(result);
                List<string> folders = JsonConvert.DeserializeObject<List<string>>(json.SelectToken("list").ToString());
                FoldersListBox.ItemsSource = folders;
            }
            catch { }

        }

        /// <summary>
        /// Get a list of the saved log files under the specified log folder via direct method.
        /// </summary>
        private async void GetFilesButton_ClickAsync(object sender, RoutedEventArgs e)
        {
            string result = await _mainPage.CallDeviceMethod("windows.enumDMFiles", "{\"folder\" : \"" + SelectedFolderInput.Text + "\",}");
            try
            {
                JObject json = JObject.Parse(result);
                List<string> files = JsonConvert.DeserializeObject<List<string>>(json.SelectToken("list").ToString());
                FilesListBox.ItemsSource = files;
            }
            catch { }

        }

        /// <summary>
        /// Update UI with selected folder.
        /// </summary>
        private void ListBoxFolder_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            ListBox listBox = (ListBox)sender;
            if (listBox != null && listBox.SelectedItem != null)
                SelectedFolderInput.Text = listBox.SelectedItem.ToString();
        }

        /// <summary>
        /// Update UI with selected file.
        /// </summary>
        private void ListBoxFiles_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            ListBox listBox = (ListBox)sender;
            if (listBox != null && listBox.SelectedItem != null)
                SelectedFileInput.Text = listBox.SelectedItem.ToString();
        }

        /// <summary>
        /// Upload a saved file to Azure Storage via direct method.
        /// </summary>
        private void UploadFileButton_Click(object sender, RoutedEventArgs e)
        {
            if (App.STORAGECONNSTRING.Length == 0)
            {
                _mainPage.ShowDialogAsync("Missing Connection String", "Please enter the Storage Connection String in Settings");
                return;
            }

            string folderName = SelectedFolderInput.Text;
            string fileName = SelectedFileInput.Text;
            string containerName = StorageContainerInput.Text;
            if (folderName.Length == 0 || fileName.Length == 0 || containerName.Length == 0)
            {
                _mainPage.ShowDialogAsync("Invaid Input", "Please enter all fields to upload log file to Azure Storage");
                return;
            }

            string uploadInput = String.Format("\"folder\" : \"{0}\",\"fileName\" : \"{1}\",\"connectionString\": \"{2}\",\"container\": \"{3}\"",
                folderName, fileName, App.STORAGECONNSTRING, containerName);
            var result = _mainPage.CallDeviceMethod("windows.uploadDMFile", "{" + uploadInput + "}");

        }

        /// <summary>
        /// Delete a saved file to from device storage via direct method.
        /// </summary>
        private void DeleteFileButton_Click(object sender, RoutedEventArgs e)
        {
            string folderName = SelectedFolderInput.Text;
            string fileName = SelectedFileInput.Text;
            if (folderName.Length == 0 || fileName.Length == 0)
            {
                _mainPage.ShowDialogAsync("Invaid Input", "Please select Log Folder, and Log File to delete a Log File");
                return;
            }

            string deleteInput = String.Format("\"folder\" : \"{0}\",\"fileName\" : \"{1}\"", folderName, fileName);
            var result = _mainPage.CallDeviceMethod("windows.deleteDMFile", "{" + deleteInput + "}");
        }

        /// <summary>
        /// Update collector list view with json parameter. 
        /// </summary>
        /// <param name="token">The json object that describes the collector list.</param>
        private void EventTracingJsonToUI(JToken twinResult)
        {
            if (!(twinResult is JObject))
            {
                _mainPage.ShowDialogAsync("Read Diagnistic Log Twin Error", "Invalid apps node json format!");
                return;
            }

            List<CollectorReported> collectorList = new List<CollectorReported>();
            List<EventTracingDataContract.CollectorInner> collectorInners = new List<EventTracingDataContract.CollectorInner>();
            string lastChangeTime = string.Empty;

            JObject root = (JObject)twinResult;
            foreach (JToken p in root.Children())
            {
                if (!(p is JProperty))
                {
                    continue;
                }

                JProperty childProperty = (JProperty)p;
                if (childProperty.Name == "lastChange")
                {
                    var timeToken = childProperty.Value.SelectToken("time");
                    if (timeToken != null)
                    {
                        lastChangeTime = "Last Update: " + timeToken.ToString();
                    }
                }
                else
                {
                    CollectorReported collectorReported = new CollectorReported();

                    collectorReported.Name = childProperty.Name;
                    EventTracingDataContract.CollectorInner collectorInner = EventTracingDataContract.CollectorInner.FromJsonObject((JObject)childProperty.Value);
                    collectorReported.CollectorInners = collectorInner;
                    collectorList.Add(collectorReported);
                }
            }
            LastChangeTime.Text = lastChangeTime;
            CollectorsListView.ItemsSource = collectorList;
        }

        /// <summary>
        /// Private class to describe an Collector. 
        /// </summary>
        private class CollectorReported
        {
            public string Name { get; set; }
            public EventTracingDataContract.CollectorInner CollectorInners { get; set; }
        }

    }
}