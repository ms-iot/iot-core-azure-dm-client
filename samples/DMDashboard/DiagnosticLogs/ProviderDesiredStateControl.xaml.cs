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

using DMDataContract;
using Newtonsoft.Json.Linq;
using System;
using System.Globalization;
using System.Windows.Controls;
using System.Windows.Data;

namespace DMDashboard
{
    class LogLevelToInt : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            if (!(value is string))
                return 0;

            string stringValue = (string)value;
            int returnValue = 0;
            switch (stringValue)
            {
                case "critical":
                    returnValue = 0;
                    break;
                case "error":
                    returnValue = 1;
                    break;
                case "warning":
                    returnValue = 2;
                    break;
                case "information":
                    returnValue = 3;
                    break;
                case "verbose":
                    returnValue = 4;
                    break;
            }

            return returnValue;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            if (!(value is int))
                return "verbose";

            int intValue = (int)value;
            string returnString = "verbose";
            switch (intValue)
            {
                case 0:
                    returnString = "critical";
                    break;
                case 1:
                    returnString = "error";
                    break;
                case 2:
                    returnString = "warning";
                    break;
                case 3:
                    returnString = "information";
                    break;
                case 4:
                    returnString = "verbose";
                    break;
            }

            return returnString;
        }
    }

    public class ProviderDesiredState
    {
        public string Name { get; set; }
        public string TraceLevel { get; set; }
        public string Keywords { get; set; }
        public bool Enabled { get; set; }
        public string Type { get; set; }

        public ProviderDesiredState()
        {
            Name = "";
            TraceLevel = "verbose";
            Keywords = "Keywords";
            Enabled = true;
            Type = "provider";
        }

        public static ProviderDesiredState ProviderDesiredStateFromJson(string providerName, JToken jValueToken)
        {
            if (!(jValueToken is JObject))
            {
                return null;
            }
            JObject jValueObject = (JObject)jValueToken;


            ProviderDesiredState providerDesiredState = new ProviderDesiredState();

            providerDesiredState.Name = providerName;
            providerDesiredState.TraceLevel = Utils.GetString(jValueObject, "traceLevel", "verbose");
            providerDesiredState.Keywords = Utils.GetString(jValueObject, "keywords", "*");
            providerDesiredState.Enabled = Utils.GetBool(jValueObject, "enabled", true);
            providerDesiredState.Type = Utils.GetString(jValueObject, "type", "provider");
            return providerDesiredState;
        }
    }

    public partial class ProviderDesiredStateControl : DMControlList
    {
        public ProviderDesiredStateControl()
        {
            InitializeComponent();
        }

        public override string PropertyName
        {
            get
            {
                return ProviderGuid.PropertyValue;
            }
        }

        public override UIElementCollection Controls
        {
            get
            {
                return ControlsList.Children;
            }
        }
    }
}
