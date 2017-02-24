using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;

namespace DMDashboard
{
    public partial class CertificateSelector : UserControl
    {
        class CertificateData
        {
            public string FileName { get; set; }
            public string Hash { get; set; }
        }

        static char Separator = '/';

        public static readonly DependencyProperty ConnectionStringProperty = DependencyProperty.Register("ConnectionString", typeof(string), typeof(CertificateSelector));
        public static readonly DependencyProperty ContainerNameProperty = DependencyProperty.Register("ContainerName", typeof(string), typeof(CertificateSelector));
        public static readonly DependencyProperty HashesStringProperty = DependencyProperty.Register("FileNamesString", typeof(string), typeof(CertificateSelector));

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
            get { return (string)GetValue(HashesStringProperty); }
            set { SetValue(HashesStringProperty, value); }
        }

        public CertificateSelector()
        {
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

        private void OnCompose(object sender, RoutedEventArgs e)
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
            foreach (string fileName in azureBlobSelector.BlobFileNames)
            {
                CertificateData certificateData = new CertificateData();
                certificateData.FileName = fileName;
                certificateData.Hash = CertificateHelpers.GetCertificateInfo(ConnectionString, ContainerName, fileName, @"c:\temp\certificates").Thumbprint;
                certificateList.Add(certificateData);

                if (FileNamesString.Length != 0)
                {
                    FileNamesString += Separator;
                }
                FileNamesString += fileName;
            }

            // Update the UI
            certificateList.Sort((x, y) => x.Hash.CompareTo(y.Hash));
            CertificateList.ItemsSource = certificateList;
        }
    }
}
