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

namespace DMDataContract
{
    public class Utils
    {
        public static JToken GetJToken(JObject jObj, string propertyName)
        {
            JToken jValue;
            if (jObj.TryGetValue(propertyName, out jValue))
            {
                return jValue;
            }
            return null;
        }

        public static bool TryGetString(JObject jObj, string propertyName, out string propertyValue)
        {
            bool retValue = false;
            JToken jValue;
            propertyValue = "";

            if (jObj.TryGetValue(propertyName, out jValue))
            {
                if (jValue.Type == JTokenType.String)
                {
                    propertyValue = (string)jValue;
                    retValue = true;
                }
                else
                {
                    Debug.WriteLine($"Property {propertyName} found but its type is not string!");
                }
            }
            return retValue;
        }

        public static string GetString(JObject jObj, string propertyName, string defaultValue)
        {
            JToken jValue;
            if (jObj.TryGetValue(propertyName, out jValue))
            {
                return jValue.ToString();
            }
            return defaultValue;
        }

        public static bool GetBool(JObject jObj, string propertyName, bool defaultValue)
        {
            JToken jValue;
            if (jObj.TryGetValue(propertyName, out jValue))
            {
                if (jValue.Type == JTokenType.Boolean)
                {
                    return (bool)jValue;
                }
                else
                {
                    Debug.WriteLine($"Property {propertyName} found but its type is not boolean!");
                }
            }
            return defaultValue;
        }

        public static string[] GetStringArray(JObject jObj, string propertyName)
        {
            List<string> list = new List<string>();

            JToken jValue;
            if (jObj.TryGetValue(propertyName, out jValue))
            {
                if (jValue.Type == JTokenType.Array)
                {
                    JArray jsonArray = (JArray)jValue;
                    foreach (object o in jsonArray)
                    {
                        if (!(o is JValue))
                        {
                            continue;
                        }
                        JValue v = (JValue)o;
                        if (!(v.Value is string))
                        {
                            continue;
                        }
                        list.Add((string)v.Value);
                    }
                }
                else
                {
                    Debug.WriteLine($"Property {propertyName} found but its type is not array!");
                }
            }

            return list.ToArray();
        }
    }
}
