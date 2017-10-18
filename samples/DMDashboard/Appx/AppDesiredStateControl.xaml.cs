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
using System;
using System.Diagnostics;
using System.Globalization;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;

namespace DMDashboard
{
    public class AppDesiredState
    {
        public enum DesiredState : int
        {
            Installed = 0,
            NotInstalled = 1,
            Reported = 2,
            NotReported = 3
        };

        public enum StartupState : int
        {
            None = 0,
            Foreground = 1,
            Background = 2
        }

        public string PackageFamilyName { get; set; }
        public DesiredState State { get; set; }
        public string Version { get; set; }
        public StartupState StartUp { get; set; }
        public string AppxSource { get; set; }
        public string Dep0Source { get; set; }
        public string Dep1Source { get; set; }
        public string CertificateSource { get; set; }
        public string CertificateTarget { get; set; }

        public AppDesiredState()
        { }

        public AppDesiredState(string packageFamilyName, DesiredState state, string version, StartupState startUp, string appxSource, string dep0Source, string dep1Source, string certificateSource, string certificateTarget)
        {
            PackageFamilyName = packageFamilyName;
            State = state;
            Version = version;
            StartUp = startUp;
            AppxSource = appxSource;
            Dep0Source = dep0Source;
            Dep1Source = dep1Source;
            CertificateSource = certificateSource;
            CertificateTarget = certificateTarget;
        }
    }

    class DesiredStateToInt : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            if (!(value is AppDesiredState.DesiredState))
                return DependencyProperty.UnsetValue;

            return (int)((AppDesiredState.DesiredState)value);
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            if (!(value is int))
                return DependencyProperty.UnsetValue;

            return (AppDesiredState.DesiredState)((int)value);
        }
    }

    class StartupStateToInt : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            if (!(value is AppDesiredState.StartupState))
                return DependencyProperty.UnsetValue;

            return (int)((AppDesiredState.StartupState)value);
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            if (!(value is int))
                return DependencyProperty.UnsetValue;

            return (AppDesiredState.StartupState)((int)value);
        }
    }
    public partial class AppDesiredStateControl : UserControl
    {
        public AppDesiredStateControl()
        {
            InitializeComponent();
        }

        public string ToJson()
        {
            if (SlotUsed.IsChecked != true || String.IsNullOrEmpty(PkgFamilyName.Text))
            {
                return "";
            }

            string packageFamilyId = PkgFamilyName.Text.Replace('.', '_');

            StringBuilder sb = new StringBuilder();
            sb.Append("\"" + packageFamilyId + "\" : ");
            switch (DesiredAction.SelectedIndex)
            {
                case 0: // Desired = Installed
                {
                    sb.Append("{\n");
                    sb.Append("\"pkgFamilyName\" : \"" + PkgFamilyName.Text + "\",\n");
                    sb.Append("\"version\" : \"" + DesiredVersion.Text + "\",\n");
                    sb.Append("\"startUp\" : \"" + StartupStateToString((AppDesiredState.StartupState)StartupState.SelectedIndex) + "\",\n");
                    if (!String.IsNullOrEmpty(DesiredAppxSource.Text))
                    {
                        sb.Append("\"appxSource\" : \"" + DesiredAppxSource.Text + "\",\n");
                    }
                    string dependenciesSources = "";
                    if (!String.IsNullOrEmpty(DesiredDep0Source.Text))
                    {
                        dependenciesSources += DesiredDep0Source.Text;
                    }
                    if (!String.IsNullOrEmpty(DesiredDep1Source.Text))
                    {
                        if (!String.IsNullOrEmpty(dependenciesSources))
                        {
                            dependenciesSources += ";";
                        }
                        dependenciesSources += DesiredDep1Source.Text;
                    }
                    if (!String.IsNullOrEmpty(dependenciesSources))
                    {
                        sb.Append("\"depsSources\" : \"" + dependenciesSources + "\",\n");
                    }
                    if (!String.IsNullOrEmpty(DesiredCertificateSource.Text))
                    {
                        sb.Append("\"certSource\" : \"" + DesiredCertificateSource.Text + "\",\n");
                    }
                    if (!String.IsNullOrEmpty(DesiredCertificateTarget.Text))
                    {
                        sb.Append("\"certStore\" : \"" + DesiredCertificateTarget.Text + "\",\n");
                    }
                    sb.Append("}\n");
                }
                break;
            case 1: // Desired = Not Installed
                {
                    // Note that nulls still need to be explicitly specified since the properties below will get merged with the existing once.
                    // So, if one already exists and is not null, we want it to be forced to null.

                    sb.Append("{\n");
                    sb.Append("\"pkgFamilyName\" : \"" + PkgFamilyName.Text + "\",\n");
                    sb.Append("\"version\" : \"not installed\",\n");
                    sb.Append("\"startUp\" : \"" + StartupStateToString((AppDesiredState.StartupState)StartupState.SelectedIndex) + "\",\n");
                    sb.Append("\"appxSource\" : null,\n");
                    sb.Append("\"depsSources\" : null,\n");
                    sb.Append("\"certSource\" : null,\n");
                    sb.Append("\"certStore\" : null\n");
                    sb.Append("}\n");
                }
                break;
            case 2: // Desired = Reported
                {
                    // Note that nulls still need to be explicitly specified since the properties below will get merged with the existing once.
                    // So, if one already exists and is not null, we want it to be forced to null.

                    sb.Append("{\n");
                    sb.Append("\"pkgFamilyName\" : \"" + PkgFamilyName.Text + "\",\n");
                    sb.Append("\"version\" : \"?\",\n");
                    sb.Append("\"startUp\" : \"" + StartupStateToString((AppDesiredState.StartupState)StartupState.SelectedIndex) + "\",\n");
                    sb.Append("\"appxSource\" : null,\n");
                    sb.Append("\"depsSources\" : null,\n");
                    sb.Append("\"certSource\" : null,\n");
                    sb.Append("\"certStore\" : null\n");
                    sb.Append("}\n");
                }
                break;
            case 3: // Desired == Not Reported (a.k.a. not tracked)
                {
                    sb.Append("null\n");
                }
                break;
            default:
                {
                    throw new Exception("Unknown app configuration action.");
                }
            }

            Debug.WriteLine("AppDesiredStateControl json = \n" + sb.ToString());

            return sb.ToString();
        }

        private string StartupStateToString(AppDesiredState.StartupState startupState)
        {
            string s = "<unknown>";
            switch (startupState)
            {
                case AppDesiredState.StartupState.None:
                    s = "none";
                    break;
                case AppDesiredState.StartupState.Foreground:
                    s = "foreground";
                    break;
                case AppDesiredState.StartupState.Background:
                    s = "background";
                    break;
            }
            return s;
        }
    }
}
