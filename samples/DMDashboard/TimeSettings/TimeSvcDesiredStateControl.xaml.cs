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
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace DMDashboard
{
    public partial class TimeSvcDesiredStateControl : UserControl
    {
        public string SectionName
        {
            get
            {
                return "timeService";
            }
        }

        public TimeSvcDesiredStateControl()
        {
            InitializeComponent();
        }

        public string ToJson()
        {
            StringBuilder sb = new StringBuilder();
            sb.Append("\"" + SectionName + "\" : {\n");
            sb.Append("\"enabled\" : \"" + (DesiredTimeServiceEnabled.SelectedIndex == 0 ? "yes" : "no") + "\",\n");
            sb.Append("\"startup\" : \"" + (DesiredTimeServiceStartup.SelectedIndex == 0 ? "auto" : "manual") + "\",\n");
            sb.Append("\"started\" : \"" + (DesiredTimeServiceStarted.SelectedIndex == 0 ? "yes" : "no") + "\"\n");
            sb.Append("}");

            return sb.ToString();
        }
    }
}
