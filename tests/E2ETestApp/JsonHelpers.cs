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

namespace E2ETestApp
{
    static class JsonHelpers
    {
        public static bool TryGetString(JObject obj, string propertyName, out string propertyValue)
        {
            propertyValue = "";

            if (obj == null)
            {
                return false;
            }

            JToken token = obj[propertyName];
            if (token is JValue)
            {
                JValue jValue = (JValue)token;
                if (jValue.Type == JTokenType.String)
                {
                    propertyValue = (string)jValue;
                    return true;
                }
            }
            return false;
        }

        public static bool TryGetObject(JObject obj, string propertyName, out JObject propertyValue)
        {
            propertyValue = null;

            if (obj == null)
            {
                return false;
            }

            JToken token = obj[propertyName];
            if (token is JObject)
            {
                propertyValue = (JObject)token;
                return true;
            }
            return false;
        }

        public static bool TryGetArray(JObject obj, string propertyName, out JArray propertyValue)
        {
            propertyValue = null;

            if (obj == null)
            {
                return false;
            }

            JToken token = obj[propertyName];
            if (token is JArray)
            {
                propertyValue = (JArray)token;
                return true;
            }
            return false;
        }

        public static bool TryGetValue(JObject obj, string propertyName, out JValue propertyValue)
        {
            propertyValue = null;

            if (obj == null)
            {
                return false;
            }

            JToken token = obj[propertyName];
            if (token is JValue)
            {
                propertyValue = (JValue)token;
                return true;
            }
            return false;
        }

    }

}