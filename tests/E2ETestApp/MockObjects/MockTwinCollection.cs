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
using System.Collections;
using System.Collections.Generic;

namespace Mock.Microsoft.Azure.Devices.Client
{
    public class TwinCollection : IEnumerable
    {
        private const string VersionName = "$version";

        public TwinCollection()
        {
            _jObject = new JObject();
        }

        public TwinCollection(JObject root)
        {
            _jObject = root;
        }

        public dynamic this[string propertyName]
        {
            get { return _jObject[propertyName]; }
            set { _jObject[propertyName] = value; }
        }

        public long Version
        {
            get
            {
                JToken versionToken;
                if (!_jObject.TryGetValue(VersionName, out versionToken))
                {
                    return default(long);
                }

                return (long)versionToken;
            }
        }

        public IEnumerator GetEnumerator()
        {
            foreach (KeyValuePair<string, JToken> kvp in _jObject)
            {
                if (kvp.Key == VersionName)
                {
                    continue;
                }

                yield return new KeyValuePair<string, dynamic>(kvp.Key, this[kvp.Key]);
            }
        }

        public string ToJson()
        {
            return ToString();
        }

        public override string ToString()
        {
            return _jObject.ToString();
        }

        private JObject _jObject;
    }
}