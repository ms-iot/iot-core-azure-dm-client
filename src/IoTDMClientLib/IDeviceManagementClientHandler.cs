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
using Newtonsoft.Json.Linq;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Threading.Tasks;

namespace Microsoft.Devices.Management
{
    internal interface IClientHandlerCallBack
    {
        Task ReportPropertiesAsync(string propertyName, JToken properties);

        Task SendMessageAsync(string message, IDictionary<string, string> properties);
    }

    // Interface for handlers that implements direct methods
    internal interface IClientDirectMethodHandler
    {
        IReadOnlyDictionary<string, Func<string, Task<string>>> GetDirectMethodHandler();
    }

    // Interface for handlers that perform actions based on desired/reported properties
    internal interface IClientPropertyHandler
    {
        string PropertySectionName { get; }

        Task<DesiredPropertyApplication> OnDesiredPropertyChange(JToken value);

        Task<JObject> GetReportedPropertyAsync();
    }

    internal interface IClientPropertyDependencyHandler
    {
        string[] PropertySectionDependencyNames { get; }
        void OnDesiredPropertyDependencyChange(string section, JObject value);
    }
}
