using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;

namespace DMDashboard
{
    /// <summary>
    /// Interaction logic for CertificateExportDetails.xaml
    /// </summary>
    public partial class CertificateExportDetails : Window
    {
        public class CertificateExportDetailsData
        {
            public string ConnectionString { get; set; }
            public string ContainerName { get; set; }
            public string BlobName { get; set; }
        }

        public CertificateExportDetails()
        {
            InitializeComponent();
        }
    }
}
