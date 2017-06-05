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
using System.Diagnostics;
using System.Threading.Tasks;

namespace Microsoft.Devices.Management
{
    abstract class SectionHandler : IClientPropertyHandler
    {
        public virtual string PropertySectionName
        {
            get
            {
                throw new NotImplementedException();
            }
        }

        public virtual void OnDesiredPropertyChange(JToken desiredValue)
        {
            throw new NotImplementedException();
        }

        public virtual async Task<JObject> GetReportedPropertyAsync()
        {
            throw new NotImplementedException();
        }

        protected virtual void ApplyProperties(JToken properties)
        {
            throw new NotImplementedException();
        }

        protected virtual async Task ReportPropertiesAsync(bool reportProperties)
        {
            throw new NotImplementedException();
        }

        protected void HandleDeviceTwinControlProperties(JToken root)
        {
            bool reportProperties = false;
            Debug.WriteLine("HandleDeviceTwinControlProperties: section = " + root.ToString());
            if (root is JValue)
            {
                JValue value = (JValue)root;
                string valueString = (string)value;

                if (valueString == DMJSonConstants.NoApplyYesReportString)
                {
                    reportProperties = true;
                }
            }
            else if (root is JObject)
            {
                JObject jObject = (JObject)root;
                foreach (JToken token in jObject.Children())
                {
                    if (token is JProperty)
                    {
                        JProperty jProperty = (JProperty)token;
                        switch (jProperty.Name)
                        {
                            case DMJSonConstants.ApplyPropertiesString:
                                if (jProperty.Value is JObject)
                                {
                                    ApplyProperties(jProperty.Value);
                                }
                                else if (jProperty.Value is JValue)
                                {
                                    JValue value = (JValue)jProperty.Value;
                                    Debug.Assert((string)value.Value == DMJSonConstants.NoString);
                                }
                                break;
                            case DMJSonConstants.ReportPropertiesString:
                                if (jProperty.Value is JValue)
                                {
                                    reportProperties = (string)jProperty.Value == DMJSonConstants.YesString;
                                }
                                break;
                        }
                    }
                }

                ReportPropertiesAsync(reportProperties);
            }
        }
    }
}
