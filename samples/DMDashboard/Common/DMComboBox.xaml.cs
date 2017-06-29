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
using System.Windows;

namespace DMDashboard
{
    public partial class DMComboBox : DMControl
    {
        public static readonly DependencyProperty ValuesListProperty = DependencyProperty.Register("ValuesList", typeof(string[]), typeof(DMComboBox));
        public static readonly DependencyProperty SelectedIndexProperty = DependencyProperty.Register("SelectedIndex", typeof(int), typeof(DMComboBox));

        public string [] ValuesList
        {
            get { return (string[])GetValue(ValuesListProperty); }
            set { SetValue(ValuesListProperty, value); }
        }

        public DMComboBox()
        {
            InitializeComponent();
        }

        public override string PropertyValue
        {
            get
            {
                return (string)ValueBox.SelectedItem;
            }
            set
            {
                if (ValuesList == null)
                {
                    return;
                }
                int i = 0;
                foreach (string s in ValuesList)
                {
                    if (s == value)
                    {
                        ValueBox.SelectedIndex = i;
                        break;
                    }
                    ++i;
                }
            }
        }

        public int SelectedIndex
        {
            get
            {
                return (int)GetValue(SelectedIndexProperty);
            }

            set
            {
                SetValue(SelectedIndexProperty, (int)value);
            }
        }
    }
}
