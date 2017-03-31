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
using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.IO;
using System.Security.Cryptography.X509Certificates;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;

namespace DMDashboard
{
    public partial class CertificateSelector : UserControl
    {
        public delegate void ShowCertificateDetailsDelegate(CertificateSelector sender, CertificateData certificateData);
        public delegate void ExportCertificateDetailsDelegate(CertificateSelector sender, CertificateData certificateData);

        public class CertificateData
        {
            class CertificateJsonData
            {
                public int Status { get; set; }
                public int Tag { get; set; }
                public string base64Encoding { get; set; }
                public string issuedBy { get; set; }
                public string issuedTo { get; set; }
                public string templateName { get; set; }
                public string validFrom { get; set; }
                public string validTo { get; set; }
            }

            public string Hash { get; set; }
            public bool DetailsAvailable { get; set; }
            public string FileName { get; set; }
            public string FriendlyName { get; set; }
            public string Issuer { get; set; }
            public string Subject { get; set; }
            public int Version { get; set; }
            public DateTime IssueDate { get; set; }
            public DateTime ExpiryDate { get; set; }
            public string Base64Encoding { get; set; }
            public string TemplateName { get; set; }

            public CertificateData()
            {
                DetailsAvailable = false;
            }

            public void LoadFromCerAzureBlob(string connectionString, string containerName, string blobName, string targetFolder)
            {
                X509Certificate2 x509Certificate2 = CertificateHelpers.GetCertificateInfo(connectionString, containerName, blobName, targetFolder);
                FileName = blobName;
                Hash = x509Certificate2.Thumbprint;
                FriendlyName = x509Certificate2.FriendlyName;
                Issuer = x509Certificate2.Issuer;
                Subject = x509Certificate2.Subject;
                Version = x509Certificate2.Version;
                IssueDate = x509Certificate2.NotBefore;
                ExpiryDate = x509Certificate2.NotAfter;
                Base64Encoding = Convert.ToBase64String(x509Certificate2.GetRawCertData());
                DetailsAvailable = true;
            }

            public void LoadFromJsonAzureBlob(string connectionString, string containerName, string blobName, string targetFolder)
            {
                AzureStorageHelpers.DownloadAzureFile(connectionString, containerName, blobName, targetFolder);

                string fullFileName = targetFolder + "\\" + blobName;
                if (!File.Exists(fullFileName))
                {
                    throw new Exception("Error: failed to download certificate json file!");
                }

                string jsonString = File.ReadAllText(fullFileName);
                FileName = blobName;
                CertificateJsonData data = JsonConvert.DeserializeObject<CertificateJsonData>(jsonString);

                Issuer = data.issuedBy;
                Subject = data.issuedTo;
                IssueDate = DateTime.Parse(data.validFrom);
                ExpiryDate = DateTime.Parse(data.validTo);
                Base64Encoding = data.base64Encoding;
                TemplateName = data.templateName;
                DetailsAvailable = true;
            }

        }

        static char Separator = '/';

        public static readonly DependencyProperty ConnectionStringProperty = DependencyProperty.Register("ConnectionString", typeof(string), typeof(CertificateSelector));
        public static readonly DependencyProperty ContainerNameProperty = DependencyProperty.Register("ContainerName", typeof(string), typeof(CertificateSelector));
        public static readonly DependencyProperty FileNamesStringProperty = DependencyProperty.Register("FileNamesString", typeof(string), typeof(CertificateSelector));
        public static readonly DependencyProperty ModifyEnabledProperty = DependencyProperty.Register("EnableAddRemove", typeof(bool), typeof(CertificateSelector));
        public static readonly DependencyProperty CertificatesPathProperty = DependencyProperty.Register("CertificatesPath", typeof(string), typeof(CertificateSelector));
        public static readonly DependencyProperty EnableExportProperty = DependencyProperty.Register("EnableExport", typeof(bool), typeof(CertificateSelector));
        public static readonly DependencyProperty EnableFileNameProperty = DependencyProperty.Register("EnableFileName", typeof(bool), typeof(CertificateSelector));

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

        public bool EnableAddRemove
        {
            get { return (bool)GetValue(ModifyEnabledProperty); }
            set { SetValue(ModifyEnabledProperty, value); }
        }

        public string CertificatesPath
        {
            get { return (string)GetValue(CertificatesPathProperty); }
            set { SetValue(CertificatesPathProperty, value); }
        }

        public bool EnableExport
        {
            get { return (bool)GetValue(EnableExportProperty); }
            set { SetValue(EnableExportProperty, value); }
        }

        public bool EnableFileName
        {
            get { return (bool)GetValue(EnableFileNameProperty); }
            set { SetValue(EnableFileNameProperty, value); }
        }

        public ShowCertificateDetailsDelegate ShowCertificateDetails { get; set; }
        public ExportCertificateDetailsDelegate ExportCertificateDetails { get; set; }

        public CertificateSelector()
        {
            EnableAddRemove = true;
            EnableExport = true;
            EnableFileName = true;
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
            AzureBlobSelector azureBlobSelector = new AzureBlobSelector(currentBlobFileNames, ConnectionString, ContainerName, ".cer");
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
                    certificateData.LoadFromCerAzureBlob(ConnectionString, ContainerName, fileName, @"c:\temp\certificates");
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
                if (!certificateData.DetailsAvailable)
                {
                    string blobName = certificateData.Hash + ".json";
                    certificateData.LoadFromJsonAzureBlob(ConnectionString, ContainerName, blobName, @"c:\temp\certificates");
                }
                ShowCertificateDetails(this, certificateData);
            }
        }

        private void OnExportDetails(object sender, RoutedEventArgs e)
        {
            if (ExportCertificateDetails != null)
            {
                Button btn = (Button)sender;
                CertificateData certificateData = (CertificateData)btn.DataContext;
                ExportCertificateDetails(this, certificateData);
            }
        }
    }
}
