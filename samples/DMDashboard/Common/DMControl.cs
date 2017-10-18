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
using System.Windows;
using System.Windows.Controls;

namespace DMDashboard
{
    public class DMControl : UserControl
    {
        public static readonly DependencyProperty TitleProperty = DependencyProperty.Register("Title", typeof(string), typeof(DMControl));
        public static readonly DependencyProperty PropertyNameProperty = DependencyProperty.Register("PropertyName", typeof(string), typeof(DMControl));
        public static readonly DependencyProperty PropertyValueProperty = DependencyProperty.Register("PropertyValue", typeof(string), typeof(DMControl));
        public static readonly DependencyProperty IsNumericProperty = DependencyProperty.Register("IsNumeric", typeof(bool), typeof(DMControl));
        public static readonly DependencyProperty IsOptionalProperty = DependencyProperty.Register("IsOptional", typeof(bool), typeof(DMControl));
        public static readonly DependencyProperty IsIncludedProperty = DependencyProperty.Register("IsIncluded", typeof(bool), typeof(DMControl));

        public string Title
        {
            get { return (string)GetValue(TitleProperty); }
            set { SetValue(TitleProperty, value); }
        }

        public virtual string PropertyName
        {
            get { return (string)GetValue(PropertyNameProperty); }
            set { SetValue(PropertyNameProperty, value); }
        }

        public bool IsNumeric
        {
            get { return (bool)GetValue(IsNumericProperty); }
            set { SetValue(IsNumericProperty, value); }
        }

        public bool IsOptional
        {
            get { return (bool)GetValue(IsOptionalProperty); }
            set { SetValue(IsOptionalProperty, value); }
        }

        public bool IsIncluded
        {
            get { return (bool)GetValue(IsIncludedProperty); }
            set { SetValue(IsIncludedProperty, value); }
        }

        public DMControl()
        {
            IsIncluded = true;
        }

        public virtual string PropertyValue
        {
            get
            {
                throw new NotImplementedException();
            }
            set
            {
                throw new NotImplementedException();
            }
        }

        public virtual string ToJson()
        {
            if (!IsIncluded && IsOptional)
            {
                return "";
            }

            StringBuilder sb = new StringBuilder();
            sb.Append("\"" + PropertyName + "\" : ");
            if (IsNumeric)
            {
                sb.Append(PropertyValue);
            }
            else
            {
                sb.Append("\"" + PropertyValue + "\"");
            }
            return sb.ToString();
        }
    }
}