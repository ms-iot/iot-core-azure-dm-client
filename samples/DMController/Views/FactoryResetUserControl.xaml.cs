using Microsoft.Devices.Management.DMDataContract;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;

// The User Control item template is documented at https://go.microsoft.com/fwlink/?LinkId=234236

namespace DMController.Views
{
    public sealed partial class FactoryResetUserControl : UserControl
    {
        public FactoryResetUserControl()
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
        /// Start factory reset via direct methods.
        /// </summary>
        private void SetFactoryStatusButton_Click(object sender, RoutedEventArgs e)
        {
            var resetParam = new FactoryResetDataContract.ResetParams();
            if (RecoveryPartitionGUIDInput.Text.Length == 0)
            {
                _mainPage.ShowDialogAsync("Invaid Input", "Please enter all fields to start Factory Reset");
                return;
            }
            resetParam.recoveryPartitionGUID = RecoveryPartitionGUIDInput.Text;
            resetParam.clearTPM = ClearTPMChckbx.IsChecked == true;

            var result = _mainPage.CallDeviceMethod(FactoryResetDataContract.StartFactoryResetAsync, resetParam.ToJsonString());

        }
    }
}
