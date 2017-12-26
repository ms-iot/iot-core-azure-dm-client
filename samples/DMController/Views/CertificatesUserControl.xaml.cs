using Microsoft.Devices.Management.DMDataContract;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Linq;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;

// The User Control item template is documented at https://go.microsoft.com/fwlink/?LinkId=234236

namespace DMController.Views
{
    public sealed partial class CertificatesUserControl : UserControl
    {
        public CertificatesUserControl()
        {
            this.InitializeComponent();

            //Load certification store Combobox
            var certificateStore = Enum.GetValues(typeof(CertificateStore)).Cast<CertificateStore>();
            CertPathInstallCombobox.ItemsSource = certificateStore.ToList();
            CertPathUninstallCombobox.ItemsSource = certificateStore.ToList();
            CertPathInstallCombobox.SelectedIndex = 0;
            CertPathUninstallCombobox.SelectedIndex = 0;
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
        ///  Retrieve all certificates on device from device twin.
        /// </summary>
        private async void ListCertificateButton_ClickAsync(object sender, RoutedEventArgs e)
        {
            var twinResult = await _mainPage.GetTwinData(CertificatesDataContract.SectionName);
            if (twinResult != null)
            {
                CertificatesDataContract.ReportedProperties reportedProperties = CertificatesDataContract.ReportedProperties.FromJsonObject((JObject)twinResult);

                CertificateInfoToUI(reportedProperties.rootCATrustedCertificates_CA, "./Device/Vendor/MSFT/RootCATrustedCertificates/CA	", CertificateStore.rootCATrustedCertificates_CA, CACertsListView);
                CertificateInfoToUI(reportedProperties.rootCATrustedCertificates_Root, "./Device/Vendor/MSFT/RootCATrustedCertificates/Root", CertificateStore.rootCATrustedCertificates_Root, RootCertsListView);
                CertificateInfoToUI(reportedProperties.rootCATrustedCertificates_TrustedPublisher, "./Device/Vendor/MSFT/RootCATrustedCertificates/TrustedPublisher", CertificateStore.rootCATrustedCertificates_TrustedPublisher, TrustedPublisherCertsListView);
                CertificateInfoToUI(reportedProperties.rootCATrustedCertificates_TrustedPeople, "./Device/Vendor/MSFT/RootCATrustedCertificates/TrustedPeople", CertificateStore.rootCATrustedCertificates_TrustedPeople, TrustedPeopleCertsListView);

                CertificateInfoToUI(reportedProperties.certificateStore_CA_System, "./Vendor/MSFT/CertificateStore/CA/System", CertificateStore.certificateStore_CA_System, CASysCertsListView);
                CertificateInfoToUI(reportedProperties.certificateStore_Root_System, "./Vendor/MSFT/CertificateStore/Root/System", CertificateStore.certificateStore_Root_System, RootSysCertsListView);
                CertificateInfoToUI(reportedProperties.certificateStore_My_User, "./Vendor/MSFT/CertificateStore/My/User", CertificateStore.certificateStore_My_User, MyUserCertsListView);
                CertificateInfoToUI(reportedProperties.certificateStore_My_System, "./Vendor/MSFT/CertificateStore/My/System", CertificateStore.certificateStore_My_System, MySysCertsListView);
            }
        }

        /// <summary>
        /// A certificate is selected, update textboxes with selected certificate. 
        /// </summary>
        private void ListViewCerts_ItemClick(object sender, ItemClickEventArgs e)
        {
            CertificateData certificateData = (CertificateData)e.ClickedItem;
            if (certificateData != null)
            {
                CertHashUninstallInput.Text = certificateData.HashName;
                CertHashDetailInput.Text = certificateData.HashName;
                CertCSPPathInput.Text = certificateData.CSPPath;
                CertPathUninstallCombobox.SelectedIndex = (int)certificateData.CertificateStore;
            }
        }

        /// <summary>
        /// Install a certificate via device twin.
        /// </summary>
        private async void InstallCertButton_ClickAsync(object sender, RoutedEventArgs e)
        {
            if (App.STORAGECONNSTRING.Length == 0)
            {
                _mainPage.ShowDialogAsync("Missing Connection String", "Please enter the Storage Connection String in Settings");
                return;
            }

            string certHashInstall = CertHashInstallInput.Text;
            string certFileName = CertFileNameInput.Text;
            if (certHashInstall.Length == 0 || certFileName.Length == 0)
            {
                _mainPage.ShowDialogAsync("Invaid Input", "Please enter all fields to Install certificate");
                return;
            }

            ExternalStorageDataContract.DesiredProperties desiredProperties = new ExternalStorageDataContract.DesiredProperties();
            desiredProperties.connectionString = App.STORAGECONNSTRING;

            CertificatesDataContract.CertificateInfo certificateInfo = new CertificatesDataContract.CertificateInfo();
            certificateInfo.Hash = certHashInstall;
            certificateInfo.StorageFileName = certFileName;
            certificateInfo.State = CertificatesDataContract.JsonStateInstalled;

            CertificatesDataContract.DesiredProperties certDesiredProperties = new CertificatesDataContract.DesiredProperties();
            switch (CertPathInstallCombobox.SelectedValue)
            {
                case CertificateStore.rootCATrustedCertificates_Root:
                    certDesiredProperties.rootCATrustedCertificates_Root.Add(certificateInfo);
                    break;
                case CertificateStore.rootCATrustedCertificates_CA:
                    certDesiredProperties.rootCATrustedCertificates_CA.Add(certificateInfo);
                    break;
                case CertificateStore.rootCATrustedCertificates_TrustedPublisher:
                    certDesiredProperties.rootCATrustedCertificates_TrustedPublisher.Add(certificateInfo);
                    break;
                case CertificateStore.rootCATrustedCertificates_TrustedPeople:
                    certDesiredProperties.rootCATrustedCertificates_TrustedPeople.Add(certificateInfo);
                    break;
                case CertificateStore.certificateStore_CA_System:
                    certDesiredProperties.certificateStore_CA_System.Add(certificateInfo);
                    break;
                case CertificateStore.certificateStore_Root_System:
                    certDesiredProperties.certificateStore_Root_System.Add(certificateInfo);
                    break;
                case CertificateStore.certificateStore_My_User:
                    certDesiredProperties.certificateStore_My_User.Add(certificateInfo);
                    break;
                case CertificateStore.certificateStore_My_System:
                    certDesiredProperties.certificateStore_My_System.Add(certificateInfo);
                    break;
                default:
                    break;
            }

            string refreshingValue = "\"refreshing\"";
            string finalValue = "{" + desiredProperties.ToJsonString() + ", " + certDesiredProperties.ToJsonString() + "}";
            await _mainPage.UpdateTwinData(refreshingValue, finalValue);
        }

        /// <summary>
        /// Uninstall a certificate via device twin.
        /// </summary>
        private async void UninstallCertButton_ClickAsync(object sender, RoutedEventArgs e)
        {
            if (CertHashUninstallInput.Text.Length == 0)
            {
                _mainPage.ShowDialogAsync("Invaid Input", "Please enter all fields to uninstall certificate");
                return;
            }

            CertificatesDataContract.CertificateInfo certificateInfo = new CertificatesDataContract.CertificateInfo();
            certificateInfo.Hash = CertHashUninstallInput.Text;
            certificateInfo.State = CertificatesDataContract.JsonStateUninstalled;

            CertificatesDataContract.DesiredProperties certDesiredProperties = new CertificatesDataContract.DesiredProperties();
            switch (CertPathUninstallCombobox.SelectedValue)
            {
                case CertificateStore.rootCATrustedCertificates_Root:
                    certDesiredProperties.rootCATrustedCertificates_Root.Add(certificateInfo);
                    break;
                case CertificateStore.rootCATrustedCertificates_CA:
                    certDesiredProperties.rootCATrustedCertificates_CA.Add(certificateInfo);
                    break;
                case CertificateStore.rootCATrustedCertificates_TrustedPublisher:
                    certDesiredProperties.rootCATrustedCertificates_TrustedPublisher.Add(certificateInfo);
                    break;
                case CertificateStore.rootCATrustedCertificates_TrustedPeople:
                    certDesiredProperties.rootCATrustedCertificates_TrustedPeople.Add(certificateInfo);
                    break;
                case CertificateStore.certificateStore_CA_System:
                    certDesiredProperties.certificateStore_CA_System.Add(certificateInfo);
                    break;
                case CertificateStore.certificateStore_Root_System:
                    certDesiredProperties.certificateStore_Root_System.Add(certificateInfo);
                    break;
                case CertificateStore.certificateStore_My_User:
                    certDesiredProperties.certificateStore_My_User.Add(certificateInfo);
                    break;
                case CertificateStore.certificateStore_My_System:
                    certDesiredProperties.certificateStore_My_System.Add(certificateInfo);
                    break;
                default:
                    break;
            }

            string refreshingValue = "\"refreshing\"";
            string finalValue = "{" + certDesiredProperties.ToJsonString() + "}";
            await _mainPage.UpdateTwinData(refreshingValue, finalValue);
        }

        /// <summary>
        /// Request certificate details via direct method. 
        /// Certificate details in json file will be uploaded to Azure Storage.
        /// </summary>
        private void DetailCertButton_Click(object sender, RoutedEventArgs e)
        {
            string certHashDetail = CertHashDetailInput.Text;
            string certCspPath = CertCSPPathInput.Text;
            string containerName = ContainerNameInput.Text;
            string outputFileName = DetailOutputFilenameInput.Text;

            if (certHashDetail.Length == 0 || certCspPath.Length == 0 || containerName.Length == 0 || outputFileName.Length == 0)
            {
                _mainPage.ShowDialogAsync("Invaid Input", "Please enter all fields to get detail of Certificate");
                return;
            }
            if (App.STORAGECONNSTRING.Length == 0)
            {
                _mainPage.ShowDialogAsync("Missing Connection String", "Please enter the Storage Connection String in Settings");
                return;
            }
            string finalValue =
                "{\"path\": \"" + certCspPath +
                "\", \"hash\": \"" + certHashDetail +
                "\", \"connectionString\": \"" + App.STORAGECONNSTRING +
                "\", \"containerName\": \"" + containerName +
                "\", \"blobName\": \"" + outputFileName + ".json" +
                "\" } ";
            var result = _mainPage.CallDeviceMethod(DMJSonConstants.DTWindowsIoTNameSpace + ".getCertificateDetails", finalValue);
        }

        /// <summary>
        /// Update certificate list view.
        /// </summary>
        /// <param name="hashes">List of hashes</param>
        /// <param name="certCspPath">CSP path certificate store</param>
        /// <param name="certificateStore">Certificate store type</param>
        /// <param name="certificateListVIew">The list view object to update</param>
        private void CertificateInfoToUI(List<string> hashes, string certCspPath, CertificateStore certificateStore, ListView certificateListVIew)
        {
            if (hashes == null)
            {
                return;
            }
            hashes.Sort();
            List<CertificateData> certList = new List<CertificateData>();

            foreach (string hash in hashes)
            {
                CertificateData certificateData = new CertificateData(hash, certCspPath, certificateStore);
                certList.Add(certificateData);
            }
            certificateListVIew.ItemsSource = certList;
        }

        /// <summary>
        /// Private class to describe a Certificate. 
        /// </summary>
        private class CertificateData
        {
            public string HashName { get; set; }
            public string CSPPath { get; set; }
            public CertificateStore CertificateStore { get; set; }

            public CertificateData(string hashName, string cspPath, CertificateStore certificateStore)
            {
                HashName = hashName;
                CSPPath = cspPath;
                CertificateStore = certificateStore;
            }
        }

        /// <summary>
        /// Enum for Certificate Store types
        /// </summary>
        private enum CertificateStore
        {
            rootCATrustedCertificates_Root,
            rootCATrustedCertificates_CA,
            rootCATrustedCertificates_TrustedPublisher,
            rootCATrustedCertificates_TrustedPeople,
            certificateStore_CA_System,
            certificateStore_Root_System,
            certificateStore_My_User,
            certificateStore_My_System
        }


    }
}
