using Microsoft.Devices.Management.DMDataContract;
using Newtonsoft.Json.Linq;
using System;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;

// The User Control item template is documented at https://go.microsoft.com/fwlink/?LinkId=234236

namespace DMController.Views
{
    public sealed partial class WindowsUpdateUserControl : UserControl
    {
        public WindowsUpdateUserControl()
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
        /// Set Windows Update Policy via device twin.
        /// </summary>
        private async void SetUpdatePolicyButton_ClickAsync(object sender, RoutedEventArgs e)
        {
            JObject desiredJObj = new JObject();
            JObject propertiesJObj = new JObject();

            if (ReportedPropertiesCombobox.SelectedValue != null && ReportedPropertiesCombobox.SelectedValue.ToString().Length > 0)
            {
                desiredJObj.Add("reportProperties", ReportedPropertiesCombobox.SelectedValue.ToString());
            }
            if (RingCombobox.SelectedValue != null && RingCombobox.SelectedValue.ToString().Length > 0)
            {
                propertiesJObj.Add("ring", RingCombobox.SelectedValue.ToString());
            }
            if (PriorityCombobox.SelectedValue != null && PriorityCombobox.SelectedValue.ToString().Length > 0)
            {
                propertiesJObj.Add("sourcePriority", PriorityCombobox.SelectedValue.ToString());
            }
            int tempInt = 0;
            if (ActiveHoursStartInput.IsEnabled && Int32.TryParse(ActiveHoursStartInput.Text, out tempInt))
            {
                propertiesJObj.Add("activeHoursStart", tempInt);

            }
            if (ActiveHoursEndInput.IsEnabled && Int32.TryParse(ActiveHoursEndInput.Text, out tempInt))
            {
                propertiesJObj.Add("activeHoursEnd", tempInt);
            }
            if (AllowAutoUpdateInput.IsEnabled && Int32.TryParse(AllowAutoUpdateInput.Text, out tempInt))
            {
                propertiesJObj.Add("allowAutoUpdate", tempInt);
            }
            if (AllowUpdateServiceInput.IsEnabled && Int32.TryParse(AllowUpdateServiceInput.Text, out tempInt))
            {
                propertiesJObj.Add("allowUpdateService", tempInt);
            }
            if (BranchReadinessInput.IsEnabled && Int32.TryParse(BranchReadinessInput.Text, out tempInt))
            {
                propertiesJObj.Add("branchReadinessLevel", tempInt);
            }
            if (DeferFeatureUpdateInput.IsEnabled && Int32.TryParse(DeferFeatureUpdateInput.Text, out tempInt))
            {
                propertiesJObj.Add("deferFeatureUpdatesPeriod", tempInt);
            }
            if (DeferQualityUpdatInput.IsEnabled && Int32.TryParse(DeferQualityUpdatInput.Text, out tempInt))
            {
                propertiesJObj.Add("deferQualityUpdatesPeriod", tempInt);
            }
            if (PauseFeatureUpdateInput.IsEnabled && Int32.TryParse(PauseFeatureUpdateInput.Text, out tempInt))
            {
                propertiesJObj.Add("pauseFeatureUpdates", tempInt);
            }
            if (PauseQualityUpdateInput.IsEnabled && Int32.TryParse(PauseQualityUpdateInput.Text, out tempInt))
            {
                propertiesJObj.Add("pauseQualityUpdates", tempInt);
            }
            if (ScheduleInstallDayInput.IsEnabled && Int32.TryParse(ScheduleInstallDayInput.Text, out tempInt))
            {
                propertiesJObj.Add("scheduledInstallDay", tempInt);
            }
            if (ScheduleInstallTimeInput.IsEnabled && Int32.TryParse(ScheduleInstallTimeInput.Text, out tempInt))
            {
                propertiesJObj.Add("scheduledInstallTime", tempInt);
            }
            desiredJObj.Add("applyProperties", propertiesJObj);
            string refreshingValue = "\"refreshing\"";
            string finalValue = "{ \"" + WindowsUpdatePolicyDataContract.SectionName + "\": " + desiredJObj.ToString() + "}";
            await _mainPage.UpdateTwinData(refreshingValue, finalValue);
        }

        /// <summary>
        /// Retrieve Windows Update Policy via device twin.
        /// </summary>
        private async void GetUpdatePolicyButton_ClickAsync(object sender, RoutedEventArgs e)
        {
            var twinResult = await _mainPage.GetTwinData(WindowsUpdatePolicyDataContract.SectionName);
            if (twinResult != null)
            {
                WindowsUpdatePolicyDataContract.WUProperties reportedProperties = WindowsUpdatePolicyDataContract.WUProperties.FromJsonObject((JObject)twinResult);
                ActiveHoursStart.Text = reportedProperties.activeHoursStart.ToString();
                ActiveHoursEnd.Text = reportedProperties.activeHoursEnd.ToString();
                AllowAutoUpdate.Text = reportedProperties.allowAutoUpdate.ToString();
                AllowUpdateService.Text = reportedProperties.allowUpdateService.ToString();
                BranchReadinessLevel.Text = reportedProperties.branchReadinessLevel.ToString();
                DeferFeatureUpdatesPeriod.Text = reportedProperties.deferFeatureUpdatesPeriod.ToString();
                DeferQualityUpdatesPeriod.Text = reportedProperties.deferQualityUpdatesPeriod.ToString();
                PauseFeatureUpdates.Text = reportedProperties.pauseFeatureUpdates.ToString();
                PauseQualityUpdates.Text = reportedProperties.pauseQualityUpdates.ToString();
                ScheduledInstallDay.Text = reportedProperties.scheduledInstallDay.ToString();
                ScheduledInstallTime.Text = reportedProperties.scheduledInstallTime.ToString();
                Ring.Text = reportedProperties.ring;
                SourcePriority.Text = reportedProperties.sourcePriority;

            }
        }

        /// <summary>
        /// Check windows update input, update the UI.
        /// </summary>
        private void Checkbox_Checked(object sender, RoutedEventArgs e)
        {
            CheckBox checkBox = (CheckBox)e.OriginalSource;
            if (checkBox != null)
            {
                switch (checkBox.Name)
                {
                    case "activeHoursStartCheckbox":
                        ActiveHoursStartInput.IsEnabled = true;
                        break;
                    case "activeHoursEndCheckbox":
                        ActiveHoursEndInput.IsEnabled = true;
                        break;
                    case "allowAutoUpdateCheckbox":
                        AllowAutoUpdateInput.IsEnabled = true;
                        break;
                    case "allowUpdateServiceCheckbox":
                        AllowUpdateServiceInput.IsEnabled = true;
                        break;
                    case "branchReadinessCheckbox":
                        BranchReadinessInput.IsEnabled = true;
                        break;
                    case "deferFeatureUpdateCheckbox":
                        DeferFeatureUpdateInput.IsEnabled = true;
                        break;
                    case "deferQualityUpdateCheckbox":
                        DeferQualityUpdatInput.IsEnabled = true;
                        break;
                    case "pauseFeatureUpdateCheckbox":
                        PauseFeatureUpdateInput.IsEnabled = true;
                        break;
                    case "pauseQualityUpdateCheckbox":
                        PauseQualityUpdateInput.IsEnabled = true;
                        break;
                    case "scheduleInstallDayCheckbox":
                        ScheduleInstallDayInput.IsEnabled = true;
                        break;
                    case "scheduleInstallTimeCheckbox":
                        ScheduleInstallTimeInput.IsEnabled = true;
                        break;
                }
            }
        }

        /// <summary>
        /// Uncheck windows update input,  update the UI.
        /// </summary>
        private void Checkbox_Unchecked(object sender, RoutedEventArgs e)
        {
            CheckBox checkBox = (CheckBox)e.OriginalSource;
            if (checkBox != null)
            {
                switch (checkBox.Name)
                {
                    case "activeHoursStartCheckbox":
                        ActiveHoursStartInput.IsEnabled = false;
                        break;
                    case "activeHoursEndCheckbox":
                        ActiveHoursEndInput.IsEnabled = false;
                        break;
                    case "allowAutoUpdateCheckbox":
                        AllowAutoUpdateInput.IsEnabled = false;
                        break;
                    case "allowUpdateServiceCheckbox":
                        AllowUpdateServiceInput.IsEnabled = false;
                        break;
                    case "branchReadinessCheckbox":
                        BranchReadinessInput.IsEnabled = false;
                        break;
                    case "deferFeatureUpdateCheckbox":
                        DeferFeatureUpdateInput.IsEnabled = false;
                        break;
                    case "deferQualityUpdateCheckbox":
                        DeferQualityUpdatInput.IsEnabled = false;
                        break;
                    case "pauseFeatureUpdateCheckbox":
                        PauseFeatureUpdateInput.IsEnabled = false;
                        break;
                    case "pauseQualityUpdateCheckbox":
                        PauseQualityUpdateInput.IsEnabled = false;
                        break;
                    case "scheduleInstallDayCheckbox":
                        ScheduleInstallDayInput.IsEnabled = false;
                        break;
                    case "scheduleInstallTimeCheckbox":
                        ScheduleInstallTimeInput.IsEnabled = false;
                        break;
                }
            }
        }

        /// <summary>
        /// Set Windows Updates via device twin.
        /// </summary>
        private async void SetWindowsUpdateButton_ClickAsync(object sender, RoutedEventArgs e)
        {
            WindowsUpdatesDataContract.DesiredProperties desiredProperties = new WindowsUpdatesDataContract.DesiredProperties();
            desiredProperties.approved = WindowsUpdateInput.Text;
            string refreshingValue = "\"refreshing\"";
            string finalValue = "{" + desiredProperties.ToJsonString() + "}";
            await _mainPage.UpdateTwinData(refreshingValue, finalValue);
        }

        /// <summary>
        /// Retrieve Windows Updates via device twin.
        /// </summary>
        private async void GetWindowsUpdateButton_ClickAsync(object sender, RoutedEventArgs e)
        {
            var twinResult = await _mainPage.GetTwinData(WindowsUpdatesDataContract.SectionName);
            if (twinResult != null)
            {
                WindowsUpdatesDataContract.ReportedProperties reportedProperties = WindowsUpdatesDataContract.ReportedProperties.FromJsonObject((JObject)twinResult);
                UpdateApproved.Text = reportedProperties.approved;
                UpdateDeferUpgrade.Text = reportedProperties.deferUpgrade.ToString();
                UpdateFailed.Text = reportedProperties.failed;
                UpdateInstallable.Text = reportedProperties.installable;
                UpdateInstalled.Text = reportedProperties.installed;
                UpdateLastScanTime.Text = reportedProperties.lastScanTime;
                UpdatePendingReboot.Text = reportedProperties.pendingReboot;

            }
        }
    }
}
