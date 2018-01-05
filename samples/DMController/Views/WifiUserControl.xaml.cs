using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;

// The User Control item template is documented at https://go.microsoft.com/fwlink/?LinkId=234236

namespace DMController.Views
{
    public sealed partial class WifiUserControl : UserControl
    {
        public WifiUserControl()
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
        /// Request device to report wifi profile via device twin.
        /// </summary>
        private async void ListWifiProfilesButton_ClickAsync(object sender, RoutedEventArgs e)
        {
            string refreshingValue = "\"refreshing\"";
            string finalValue =
                "{ \"wifi\" : \"no-apply-yes-report\" }";
            await _mainPage.UpdateTwinData(refreshingValue, finalValue);
        }

        private void ListViewProfiles_ItemClick(object sender, ItemClickEventArgs e)
        {
            //Todo
        }

        /// <summary>
        /// Install a wifi profile via device twin.
        /// </summary>
        private async void InstallProfileButton_ClickAsync(object sender, RoutedEventArgs e)
        {
            string installProfile = InstallProfileInput.Text;
            string installPath = InstallPathInput.Text;

            if (installProfile.Length == 0 || installPath.Length == 0)
            {
                _mainPage.ShowDialogAsync("Invaid Input", "Please enter all fields to Install Wifi Profile");
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
                "\"},  \"wifi\" : { \"applyProperties\" : { \"" + installProfile +
                "\" : { \"profile\": \"" + installPath +
                "\" }}}}";
            await _mainPage.UpdateTwinData(refreshingValue, finalValue);
        }

        /// <summary>
        /// Unistall a wifi profile via device twin.
        /// </summary>
        private async void UninstallProfileButton_ClickAsync(object sender, RoutedEventArgs e)
        {
            string uninstallProfile = UninstallProfileInput.Text;

            if (uninstallProfile.Length == 0)
            {
                _mainPage.ShowDialogAsync("Invaid Input", "Please enter all fields to Uninstall Wifi Profile");
                return;
            }
            if (App.STORAGECONNSTRING.Length == 0)
            {
                _mainPage.ShowDialogAsync("Missing Connection String", "Please enter the Storage Connection String in Settings");
                return;
            }
            string refreshingValue = "\"refreshing\"";
            string finalValue =
                "{ \"wifi\" : { \"applyProperties\" : { \"" + uninstallProfile +
                "\" : \"uninstall\" }}}";
            await _mainPage.UpdateTwinData(refreshingValue, finalValue);
        }

    }
}
