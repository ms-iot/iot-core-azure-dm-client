/*
Copyright 2018 Microsoft
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

namespace DMValidator
{
    class TestParameters
    {
        const string JsonAzureStorageConnectionStringHint = "<azure storage connection string>";
        const string JsonAzureStorageConnectionStringValue = "<replace with the real azure storage connection string>";

        public string IoTHubConnectionString { get; set; }
        public string IoTHubDeviceId { get; set; }

        public TestParameters()
        {
            _dictionary = new Dictionary<string, string>();
            _dictionary[JsonAzureStorageConnectionStringHint] = JsonAzureStorageConnectionStringValue;
        }

        private string Resolve(string key)
        {
            string value;
            if (_dictionary.TryGetValue(key, out value))
            {
                return value;
            }
            return key;
        }

        private void ResolveParametersInternal(JToken root)
        {
            if (root is JObject)
            {
                JObject jObject = (JObject)root;
                foreach (JProperty childProperty in jObject.Children())
                {
                    ResolveParametersInternal(childProperty);
                }
            }

            if (root is JProperty)
            {
                JProperty jProperty = (JProperty)root;
                if (jProperty.Value is JValue && jProperty.Value.Type == JTokenType.String)
                {
                    jProperty.Value = new JValue(Resolve((string)jProperty.Value));
                }
                else if (jProperty.Value is JObject)
                {
                    ResolveParametersInternal(jProperty.Value);
                }
            }
        }

        public JToken ResolveParameters(JToken root)
        {
            JToken resolvedTree = root.DeepClone();
            ResolveParametersInternal(resolvedTree);
            return resolvedTree;
        }

        private Dictionary<string, string> _dictionary;
    }

}