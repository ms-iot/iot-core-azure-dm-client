using Microsoft.Devices.Management.DMDataContract;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;

// The User Control item template is documented at https://go.microsoft.com/fwlink/?LinkId=234236

namespace DMController.Views
{
    public sealed partial class ApplicationsUserControl : UserControl
    {

        public ApplicationsUserControl()
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
        /// Request device to report all store and non-store installed applications via device twin.
        /// </summary>
        private async void RequestAppsButton_ClickAsync(object sender, RoutedEventArgs e)
        {
            string refreshingValue = "\"refreshing\"";
            string finalValue = "{ \"apps\" : { \"?\": {\"store\": true, \"nonStore\": true  } } }";
            await _mainPage.UpdateTwinData(refreshingValue, finalValue);
        }

        /// <summary>
        /// Retrieve all applications on device from device twin.
        /// </summary>
        private async void ListAppsButton_ClickAsync(object sender, RoutedEventArgs e)
        {
            var twinResult = await _mainPage.GetTwinData("apps");
            if (twinResult != null)
            {
                AppsStatusJsonToUI(twinResult);
            }
        }

        /// <summary>
        /// An application is selected, update textboxes with selected application. 
        /// </summary>
        private void ListViewApps_ItemClick(object sender, ItemClickEventArgs e)
        {
            AppReportedState appReportedState = (AppReportedState)e.ClickedItem;
            if (appReportedState.PackageFamilyName != null)
            {
                PackageFamilyInput1.Text = appReportedState.PackageFamilyName;
                PackageFamilyInput2.Text = appReportedState.PackageFamilyName;
                VersionInput.Text = "not installed";
                StartupInput.Text = appReportedState.StartUp;
            }
        }

        /// <summary>
        /// Add / Edit an application via device twin.
        /// </summary>
        private async void EditAppButton_ClickAsync(object sender, RoutedEventArgs e)
        {
            string packageFamily = AppPackageInput.Text;
            string version = AppVersionInput.Text;
            string startup = (AppStartupCombox.SelectedValue != null) ? AppStartupCombox.SelectedValue.ToString() : "none";
            string appxSource = AppAppxSourceInput.Text;
            string depsSource = AppDepsSourceInput.Text;
            string certSource = AppCertSourceInput.Text;
            string certStore = AppCertStoreInput.Text;
            if (packageFamily.Length == 0 || version.Length == 0 || startup.Length == 0 || appxSource.Length == 0 || depsSource.Length == 0 || certSource.Length == 0 || certStore.Length == 0)
            {
                _mainPage.ShowDialogAsync("Invaid Input", "Please enter all fields to Install or Update application");
                return;
            }
            if (App.STORAGECONNSTRING.Length == 0)
            {
                _mainPage.ShowDialogAsync("Missing Connection String", "Please enter the Storage Connection String in Settings");
                return;
            }
            string refreshingValue = "\"refreshing\"";
            string finalValue =
                "{ \"externalStorage\" : {  \"connectionString\": \"" + App.STORAGECONNSTRING +
                "\"},  \"apps\" : { \"" + packageFamily.Replace('.', '_') +
                "\": {\"pkgFamilyName\": \"" + packageFamily +
                "\", \"version\": \"" + version +
                "\", \"startUp\": \"" + startup +
                "\", \"appxSource\": \"" + appxSource +
                "\", \"depsSources\": \"" + depsSource +
                "\", \"certSource\":\"" + certSource +
                "\",  \"certStore\": \"" + certStore +
                "\" } } }";
            await _mainPage.UpdateTwinData(refreshingValue, finalValue);
        }

        /// <summary>
        /// Start / Stop an application via direct method. 
        /// </summary>
        private void StartStopAppButton_Click(object sender, RoutedEventArgs e)
        {
            if (PackageFamilyInput1.Text.Length == 0 || StartStopCombobox.SelectedValue == null)
            {
                _mainPage.ShowDialogAsync("Invaid Input", "Please enter all fields to Start or Stop application");
                return;
            }

            AppxLifeCycleDataContract.ManageAppLifeCycleParams parameters = new AppxLifeCycleDataContract.ManageAppLifeCycleParams();
            parameters.action = AppxLifeCycleDataContract.JsonStart;
            parameters.pkgFamilyName = PackageFamilyInput1.Text;
            if (StartStopCombobox.SelectedValue.ToString() == "start")
            {
                parameters.action = AppxLifeCycleDataContract.JsonStart;
            }
            else if (StartStopCombobox.SelectedValue.ToString() == "stop")
            {
                parameters.action = AppxLifeCycleDataContract.JsonStop;
            }
            else
            {
                _mainPage.ShowDialogAsync("Invaid Input", "Please enter all fields to Start or Stop application");
                return;
            }
            var result = _mainPage.CallDeviceMethod(AppxLifeCycleDataContract.ManageAppLifeCycleAsync, parameters.ToJsonString());
        }

        /// <summary>
        /// Uninstall an application via device twin.
        /// </summary>
        private async void UninstallAppButton_ClickAsync(object sender, RoutedEventArgs e)
        {
            if (PackageFamilyInput2.Text.Length == 0)
            {
                _mainPage.ShowDialogAsync("Invaid Input", "Please enter all fields to Uninstall application");
                return;
            }
            string refreshingValue = "\"refreshing\"";
            AppReportedState[] appArray = (AppReportedState[])AppsListView.ItemsSource;
            if (appArray != null && appArray.Length > 0)
            {
                AppReportedState appSelected = Array.Find(appArray, a => a.PackageFamilyName == PackageFamilyInput2.Text);
                string finalValue = "{ \"apps\" : { \"" + appSelected.PackageFamilyName.Replace('.', '_') + "\": {\"pkgFamilyName\": \"" + appSelected.PackageFamilyName + "\", \"version\": \"" + VersionInput.Text + "\", \"startup\": \"" + StartupInput.Text + "\", \"installDate\": \"" + appSelected.InstallDate + "\" } } }";
                await _mainPage.UpdateTwinData(refreshingValue, finalValue);
            }
        }

        /// <summary>
        /// Update UI with selected architecture type. 
        /// </summary>
        private void InstallAppArch_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            ComboBox comboBox = (ComboBox)sender;
            if (comboBox != null)
            {
                if (comboBox.SelectedValue.ToString() == "ARM")
                {
                    AppPackageInput.Text = "DMClientOverview_jv0mnc6v5g12j";
                    AppAppxSourceInput.Text = "dmappx-arm\\\\DMClientOverview_1.0.0.0_ARM_Debug.appx";
                    AppDepsSourceInput.Text = "dmappx-arm\\\\Microsoft.NET.CoreRuntime.1.1.appx;dmappx-arm\\\\Microsoft.VCLibs.ARM.Debug.14.00.appx";
                    AppCertSourceInput.Text = "dmappx-arm\\\\DMClientOverview_1.0.0.0_ARM_Debug.cer";
                    AppCertStoreInput.Text = "./Device/Vendor/MSFT/RootCATrustedCertificates/TrustedPeople";
                }
                else if (comboBox.SelectedValue.ToString() == "x64")
                {
                    AppPackageInput.Text = "DMClientOverview_jv0mnc6v5g12j";
                    AppAppxSourceInput.Text = "dmappx-x64\\\\DMClientOverview_1.0.0.0_x64_Debug.appx";
                    AppDepsSourceInput.Text = "dmappx-x64\\\\Microsoft.NET.CoreRuntime.1.1.appx;dmappx-x64\\\\Microsoft.VCLibs.x64.Debug.14.00.appx";
                    AppCertSourceInput.Text = "dmappx-x64\\\\DMClientOverview_1.0.0.0_x64_Debug.cer";
                    AppCertStoreInput.Text = "./Device/Vendor/MSFT/RootCATrustedCertificates/TrustedPeople";
                }
            }

        }

        /// <summary>
        /// Update Applications list view with json parameter. 
        /// </summary>
        /// <param name="token">The json object that describes the application list.</param>
        private void AppsStatusJsonToUI(JToken token)
        {
            if (!(token is JObject))
            {
                _mainPage.ShowDialogAsync("Read Application Twin Error", "Invalid apps node json format!");
                return;
            }

            List<AppReportedState> data = new List<AppReportedState>();

            JObject root = (JObject)token;
            foreach (JToken p in root.Children())
            {
                if (!(p is JProperty))
                {
                    continue;
                }
                JProperty property = (JProperty)p;
                string packageFamilyJsonId = property.Name;
                if (property.Value == null)
                {
                    data.Add(new AppReportedState(packageFamilyJsonId, null, null, null, null));
                }
                else if (property.Value is JObject)
                {
                    AppReportedState appStatusData = new AppReportedState();
                    JObject packageFamilyProperties = (JObject)property.Value;
                    foreach (JToken p1 in packageFamilyProperties.Children())
                    {
                        if (!(p1 is JProperty))
                        {
                            continue;
                        }
                        JProperty childProperty = (JProperty)p1;
                        if (childProperty.Name == "pkgFamilyName")
                        {
                            appStatusData.PackageFamilyName = childProperty.Value.ToString();
                        }
                        else if (childProperty.Name == "version")
                        {
                            appStatusData.Version = childProperty.Value.ToString();
                        }
                        else if (childProperty.Name == "installDate")
                        {
                            appStatusData.InstallDate = childProperty.Value.ToString();
                        }
                        else if (childProperty.Name == "startUp")
                        {
                            appStatusData.StartUp = childProperty.Value.ToString();
                        }
                        else if (childProperty.Name == "error")
                        {
                            appStatusData.Error = childProperty.Value.ToString();
                        }
                    }
                    if (appStatusData.PackageFamilyName != null && appStatusData.PackageFamilyName.Length > 0)
                        data.Add(appStatusData);
                }
            }
            AppsListView.ItemsSource = data.ToArray();
        }

        /// <summary>
        /// Private class to describe an Application. 
        /// </summary>
        private class AppReportedState
        {
            public string PackageFamilyName { get; set; }
            public string Version { get; set; }
            public string InstallDate { get; set; }
            public string StartUp { get; set; }
            public string Error { get; set; }

            public AppReportedState() { }

            public AppReportedState(string packageFamilyName, string version, string installDate, string startUp, string error)
            {
                PackageFamilyName = packageFamilyName;
                Version = version;
                InstallDate = installDate;
                StartUp = startUp;
                Error = error;
            }
        }


    }

}
