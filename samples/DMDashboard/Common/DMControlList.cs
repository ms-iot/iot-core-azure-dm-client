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
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;

namespace DMDashboard
{
    public class DMControlList : DMControl
    {
        public virtual UIElementCollection Controls
        {
            get
            {
                throw new NotImplementedException();
            }
        }

        public override string ToJson()
        {
            if (!base.IsIncluded && base.IsOptional)
            {
                return "";
            }

            StringBuilder sb = new StringBuilder();
            foreach (UIElement uc in Controls)
            {
                if (!(uc is DMControl))
                {
                    continue;
                }

                DMControl dmControl = (DMControl)uc;
                string controlJson = dmControl.ToJson();

                if (!String.IsNullOrEmpty(controlJson))
                {
                    if (sb.Length > 0)
                    {
                        sb.Append(",\n");
                    }
                    sb.Append(controlJson);
                }
            }

            StringBuilder asb = new StringBuilder();
            if (PropertyName.Length > 0)
            {
                asb.Append("\"" + PropertyName + "\" : ");
                asb.Append("{\n");
                asb.Append(sb.ToString());
                asb.Append("}\n");
            }
            else
            {
                asb.Append(sb.ToString());
            }

            return asb.ToString();
        }
    }
}
