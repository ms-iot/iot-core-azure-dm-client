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
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading.Tasks;
using Windows.Foundation.Diagnostics;

using Mock.App;
using Mock.Microsoft.Azure.Devices.Client;

namespace E2ETestApp
{
    class TestCase
    {
        private const string JsonReported = "reported";
        private const string JsonProperties = "properties";

        public string Name
        {
            get
            {
                return _name;
            }
        }

        public virtual void Dump()
        {
        }

        public void Initialize(Twin twin, DeviceClientApp app, RegistryManager deviceController)
        {
            _twin = twin;
            _app = app;
            _deviceController = deviceController;

            _deviceController.SetReportedPropertyUpdateCallbackAsync(OnReportedPropertyUpdate);
        }

        private Task OnReportedPropertyUpdate(TwinCollection reportedProperties)
        {
            Debug.WriteLine("Test - OnReportedPropertyUpdate() -- Received Start ----------------------------------");
            Debug.WriteLine(reportedProperties.ToString());
            Debug.WriteLine("Test - OnReportedPropertyUpdate() -- Received End ------------------------------------");

            if (_actualReportedState == null)
            {
                // Create new...

                JObject reportedObject = new JObject();
                foreach (KeyValuePair<string, object> pair in reportedProperties)
                {
                    reportedObject[pair.Key] = (JToken)pair.Value;
                }

                JObject propertiesObject = new JObject();
                propertiesObject.Add(JsonReported, reportedObject);

                JObject rootObject = new JObject();
                rootObject.Add(JsonProperties, propertiesObject);

                _actualReportedState = rootObject;
            }
            else
            {
                // Merge to existing...

                JObject reportedObject = new JObject();
                foreach (KeyValuePair<string, object> pair in reportedProperties)
                {
                    reportedObject[pair.Key] = (JToken)pair.Value;
                }

                JObject reported = (JObject)_actualReportedState[JsonProperties][JsonReported];

                /*
                Debug.WriteLine("-- Merging ----------------------");
                Debug.WriteLine("-- -- Base:");
                Debug.WriteLine(reported);
                Debug.WriteLine("-- -- New:");
                Debug.WriteLine(reportedObject);
                */

                reported.Merge(reportedObject);

                /*
                Debug.WriteLine("-- -- Result:");
                Debug.WriteLine(reported);
                Debug.WriteLine("-- -- --");
                */
            }

            return Task.CompletedTask;
        }

        public virtual Task<bool> Execute(ILogger logger, TestParameters testParameters)
        {
            return Task.FromResult<bool>(true);
        }

        protected static void ReportError(ILogger logger, string msg)
        {
            Debug.WriteLine(msg);
            logger.Log(LoggingLevel.Error, msg);
        }

        protected void ReportResult(ILogger logger, bool result, IEnumerable<string> errorList)
        {
            if (result)
            {
                logger.Log(LoggingLevel.Information, "              Case " + _name + " succeeded!");
            }
            else
            {
                logger.Log(LoggingLevel.Information, "              Case " + _name + " failed!");
                foreach (string errorMsg in errorList)
                {
                    logger.Log(LoggingLevel.Error, "                " + errorMsg);
                }
            }
        }

        // Common properties...
        protected string _name;
        protected string _description;
        
        // Pointers to the current active mock objects...
        protected Twin _twin;
        protected DeviceClientApp _app;
        protected RegistryManager _deviceController;

        // Collected reported state...
        protected JObject _actualReportedState;
    }
}