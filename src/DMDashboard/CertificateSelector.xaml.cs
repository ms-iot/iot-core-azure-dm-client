using System;
using System.Collections.Generic;
using System.Security.Cryptography.X509Certificates;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;

namespace DMDashboard
{
    public partial class CertificateSelector : UserControl
    {
        public delegate void ShowCertificateDetailsDelegate(CertificateSelector sender, CertificateData certificateData);

        public class CertificateData
        {
            public string Hash { get; set; }
            public bool DetailsAvailable { get; set; }
            public string FileName { get; set; }
            public string FriendlyName { get; set; }
            public string Issuer { get; set; }
            public string Subject { get; set; }
            public int Version { get; set; }
            public DateTime IssueDate { get; set; }
            public DateTime ExpiryDate { get; set; }

            public CertificateData()
            {
                DetailsAvailable = false;
            }
        }

        static char Separator = '/';

        public static readonly DependencyProperty ConnectionStringProperty = DependencyProperty.Register("ConnectionString", typeof(string), typeof(CertificateSelector));
        public static readonly DependencyProperty ContainerNameProperty = DependencyProperty.Register("ContainerName", typeof(string), typeof(CertificateSelector));
        public static readonly DependencyProperty FileNamesStringProperty = DependencyProperty.Register("FileNamesString", typeof(string), typeof(CertificateSelector));
        public static readonly DependencyProperty ModifyEnabledProperty = DependencyProperty.Register("ModifyEnabled", typeof(bool), typeof(CertificateSelector));
        public static readonly DependencyProperty CertificatesPathProperty = DependencyProperty.Register("CertificatesPath", typeof(string), typeof(CertificateSelector));

        public string ConnectionString
        {
            get { return (string)GetValue(ConnectionStringProperty); }
            set { SetValue(ConnectionStringProperty, value); }
        }

        public string ContainerName
        {
            get { return (string)GetValue(ContainerNameProperty); }
            set { SetValue(ContainerNameProperty, value); }
        }

        public string FileNamesString
        {
            get { return (string)GetValue(FileNamesStringProperty); }
            set { SetValue(FileNamesStringProperty, value); }
        }

        public bool ModifyEnabled
        {
            get { return (bool)GetValue(ModifyEnabledProperty); }
            set { SetValue(ModifyEnabledProperty, value); }
        }

        public string CertificatesPath
        {
            get { return (string)GetValue(CertificatesPathProperty); }
            set { SetValue(CertificatesPathProperty, value); }
        }

        public ShowCertificateDetailsDelegate ShowCertificateDetails { get; set; }

        public CertificateSelector()
        {
            ModifyEnabled = true;
            InitializeComponent();
        }

        private System.Windows.Window FindParentWindow()
        {
            DependencyObject newParent = null;
            do
            {
                newParent = VisualTreeHelper.GetParent(newParent == null ? this : newParent);
                if (newParent == null || newParent is System.Windows.Window)
                {
                    break;
                }
            } while (newParent != null);

            return newParent as System.Windows.Window;
        }

        private void OnAddRemove(object sender, RoutedEventArgs e)
        {
            Window parentWindow = FindParentWindow();
            if (parentWindow == null)
            {
                throw new Exception("Error: CertificateSelector must be a child of a Window.");
            }

            // Gather the currently 'desired' certificate file names.
            List<string> currentBlobFileNames = new List<string>();
            if (CertificateList.ItemsSource != null)
            {
                foreach (CertificateData certificateData in CertificateList.ItemsSource)
                {
                    currentBlobFileNames.Add(certificateData.FileName);
                }
            }

            // Show contents of the container, and make the ones in currentBlobFileNames as selected.
            AzureBlobSelector azureBlobSelector = new AzureBlobSelector(currentBlobFileNames, ConnectionString, ContainerName);
            azureBlobSelector.Owner = parentWindow;
            azureBlobSelector.ShowDialog();

            // Construct the new FileNamesString
            FileNamesString = "";
            List<CertificateData> certificateList = new List<CertificateData>();
            if (azureBlobSelector.BlobFileNames != null)
            {
                foreach (string fileName in azureBlobSelector.BlobFileNames)
                {
                    CertificateData certificateData = new CertificateData();
                    certificateData.FileName = fileName;

                    X509Certificate2 x509Certificate2 = CertificateHelpers.GetCertificateInfo(ConnectionString, ContainerName, fileName, @"c:\temp\certificates");
                    certificateData.Hash = x509Certificate2.Thumbprint;
                    certificateData.FriendlyName = x509Certificate2.FriendlyName;
                    certificateData.Issuer = x509Certificate2.Issuer;
                    certificateData.Subject = x509Certificate2.Subject;
                    certificateData.Version = x509Certificate2.Version;
                    certificateData.IssueDate = x509Certificate2.NotBefore;
                    certificateData.ExpiryDate = x509Certificate2.NotAfter;
                    certificateData.DetailsAvailable = true;
                    certificateList.Add(certificateData);

                    if (FileNamesString.Length != 0)
                    {
                        FileNamesString += Separator;
                    }
                    FileNamesString += fileName;
                }
            }

            // Update the UI
            certificateList.Sort((x, y) => x.Hash.CompareTo(y.Hash));
            CertificateList.ItemsSource = certificateList;
        }

        public void SetCertificateList(IEnumerable<CertificateData> certificateList)
        {
            CertificateList.ItemsSource = certificateList;
        }

        private void OnShowDetails(object sender, RoutedEventArgs e)
        {
            if (ShowCertificateDetails != null)
            {
                Button btn = (Button)sender;
                CertificateData certificateData = (CertificateData)btn.DataContext;
                ShowCertificateDetails(this, certificateData);
            }

        }
    }
}
