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
using System.Text;
using System.Windows.Controls;

namespace DMDashboard
{
    public partial class AppsDesiredStateControl : UserControl
    {
        public AppDesiredState[] AppsConfigurations
        {
            set
            {
                AppDesiredState[] states = (AppDesiredState[])value;
                AppDesiredStateControl[] controls = { App0, App1, App2, App3 };
                for (int i = 0; i < states.Length; ++i)
                {
                    controls[i].DataContext = states[i];
                }
                for (int i = states.Length; i < 4; ++i)
                {
                    controls[i].DataContext = new AppDesiredState();
                }
            }
        }

        public AppsDesiredStateControl()
        {
            InitializeComponent();
        }

        public string GetJSon()
        {
            string[] ss = {
                            App0.GetJSon(),
                            App1.GetJSon(),
                            App2.GetJSon(),
                            App3.GetJSon()
            };

            StringBuilder sb = new StringBuilder();
            foreach (string s in ss)
            {
                if (!String.IsNullOrEmpty(s))
                {
                    if (sb.Length > 0)
                    {
                        sb.Append(",\n");
                    }
                    sb.Append(s);
                }
            }

            if (sb.Length == 0)
            {
                return "\"apps\" : null";
            }

            return "\"apps\" : {\n" + sb.ToString() + "\n}";
        }
    }
}
