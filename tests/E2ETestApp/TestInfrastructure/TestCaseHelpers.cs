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
using System.Collections.Generic;
using System.Diagnostics;
using System.Text.RegularExpressions;

namespace E2ETestApp
{
    static class TestCaseHelpers
    {
        private const string JsonIgnore = "<ignore>";

        private static bool VerifyObjectPropertiesPresent(JObject expected, JObject actual, List<string> errorList)
        {
            bool result = true;

            foreach (JToken expectedChild in expected.Children())
            {
                if (!(expectedChild is JProperty))
                {
                    continue;
                }

                JProperty expectedProperty = (JProperty)expectedChild;
                JToken expectedValue = expectedProperty.Value;

                // Make sure a property with the same name exists in 'actual'...

                JToken actualValue;
                if (!actual.TryGetValue(expectedProperty.Name, out actualValue))
                {
                    string msg = "Missing node in actual [" + expectedProperty.Name + "]";
                    errorList.Add(msg);
                    Debug.WriteLine(msg);
                    result = false;
                    continue;
                }

                // If the expected property type is string, it might be an 'ignore' property...
                
                if (expectedValue.Type == JTokenType.String)
                {
                    if ((string)expectedValue == JsonIgnore)
                    {
                        Debug.WriteLine("Node marked for ignoring value. Skipping...");
                        continue;
                    }
                }

                result &= VerifyPropertiesPresent(expectedProperty.Name, expectedValue, actualValue, errorList);
            }

            return result;
        }

        private static bool VerifyArrayElementsPresent(JArray expected, JArray actual, List<string> errorList)
        {
            HashSet<string> expectedStrings = new HashSet<string>();
            foreach (JToken expectedChild in expected.Children())
            {
                if (expectedChild.Type != JTokenType.String)
                {
                    // We currently support string arrays only.
                    continue;
                }

                expectedStrings.Add((string)expectedChild);
            }

            HashSet<string> actualStrings = new HashSet<string>();
            foreach (JToken actualChild in actual.Children())
            {
                if (actualChild.Type != JTokenType.String)
                {
                    // We currently support string arrays only.
                    continue;
                }

                actualStrings.Add((string)actualChild);
            }

            bool result = true;

            foreach (string expectedString in expectedStrings)
            {
                if (!actualStrings.Contains(expectedString))
                {
                    string msg = "Missing string in actual [" + expectedString + "]";
                    errorList.Add(msg);
                    Debug.WriteLine(msg);

                    result = false;
                }
            }

            return result;
        }

        public static bool VerifyPropertiesPresent(string propertyName, JToken expectedValue, JToken actualValue, List<string> errorList)
        {
            if (expectedValue == null && actualValue != null)
            {
                string msg = "Unexpected node in actual.";
                errorList.Add(msg);
                Debug.WriteLine(msg);
                return false;
            }
            if (expectedValue != null && actualValue == null)
            {
                string msg = "Missing node in actual.";
                errorList.Add(msg);
                Debug.WriteLine(msg);
                return false;
            }
            if (expectedValue == null && actualValue == null)
            {
                return true;
            }

            // If one of the sides is a date, we'll skip the type check and try to convert to a string later...
            if (expectedValue.Type != JTokenType.Date && actualValue.Type != JTokenType.Date)
            {
                // Make sure both types agree...
                if (expectedValue.Type != actualValue.Type)
                {
                    string msg = "Mismatch token type Expected Type: " + expectedValue.Type + ", Actual Type: " + actualValue.Type + ", Expected Json: " + expectedValue.ToString();
                    errorList.Add(msg);
                    Debug.WriteLine(msg);
                    return false;
                }
            }

            bool result = true;
            string dateTimeFormat = "yyyy-MM-ddTHH:mm:ssZ";

            switch (expectedValue.Type)
            {
                case JTokenType.Object:

                    result &= VerifyObjectPropertiesPresent((JObject)expectedValue, (JObject)actualValue, errorList);
                    break;

                case JTokenType.Date:

                    string expectedDateTimeString = ((DateTime)expectedValue).ToString(dateTimeFormat);
                    string actualDateTimeString = "";
                    if (actualValue.Type == JTokenType.Date)
                    {
                        actualDateTimeString = ((DateTime)actualValue).ToString(dateTimeFormat);
                    }
                    else
                    {
                        actualDateTimeString = (string)actualValue;
                    }

                    if (expectedDateTimeString != actualDateTimeString)
                    {
                        string msg = "Mismatch: Property: " + propertyName + ", Expected Value: " + expectedDateTimeString + " ,Actual Value: " + actualDateTimeString;
                        errorList.Add(msg);
                        Debug.WriteLine(msg);
                        result = false;
                    }

                    break;

                case JTokenType.String:

                    string actualValueString = "";
                    if (actualValue.Type == JTokenType.Date)
                    {
                        actualValueString = ((DateTime)actualValue).ToString(dateTimeFormat);
                    }
                    else
                    {
                        actualValueString = (string)actualValue;
                    }


                    if ((string)expectedValue != actualValueString)
                    {
                        string msg = "Mismatch: Property: " + propertyName + ", Expected Value: " + (string)expectedValue + " ,Actual Value: " + (string)actualValue;
                        errorList.Add(msg);
                        Debug.WriteLine(msg);
                        result = false;
                    }

                    break;

                case JTokenType.Integer:
                    if ((int)expectedValue != (int)actualValue)
                    {
                        string msg = "Mismatch: Property: " + propertyName + ", Expected Value: " + (int)expectedValue + " ,Actual Value: " + (int)actualValue;
                        errorList.Add(msg);
                        Debug.WriteLine(msg);
                        result = false;
                    }

                    break;

                case JTokenType.Array:
                    if (actualValue.Type != JTokenType.Array)
                    {
                        string msg = "Mismatch: Property Type: " + propertyName + ", Expected Type: " + expectedValue.Type + " ,Actual Type: " + actualValue.Type;
                        errorList.Add(msg);
                        Debug.WriteLine(msg);
                        result = false;
                    }

                    JArray jExpectedArray = (JArray)expectedValue;
                    JArray jActualArray = (JArray)actualValue;

                    result &= VerifyArrayElementsPresent(jExpectedArray, jActualArray, errorList);

                    break;
            }

            return result;
        }

        // Used to remove the index digits from within [].
        private static string RemoveIndices(string s)
        {
            return Regex.Replace(s, @"\[\d+\]", "[]", RegexOptions.None);
        }

        private static void FlattenArray(JArray parentArray, HashSet<string> flattenedList)
        {
            bool hasElements = false;
            foreach (JToken arrayElement in parentArray.Children())
            {
                Flatten(arrayElement, flattenedList);
                hasElements = true;
            }

            // Reached a leaf node?
            if (!hasElements)
            {
                flattenedList.Add(RemoveIndices(parentArray.Path));
            }
        }

        private static void FlattenObject(JObject parentObject, HashSet<string> flattenedList)
        {
            bool hasChildren = false;
            foreach (JToken child in parentObject.Children())
            {
                if (child.Type != JTokenType.Property)
                {
                    continue;
                }
                JProperty childProperty = (JProperty)child;
                Flatten(childProperty.Value, flattenedList);

                hasChildren = true;
            }

            // Reached a leaf node?
            if (!hasChildren)
            {
                flattenedList.Add(RemoveIndices(parentObject.Path));
            }
        }

        private static void Flatten(JToken root, HashSet<string> flattenedList)
        {
            if (root.Type == JTokenType.Object)
            {
                JObject parentObject = (JObject)root;
                FlattenObject(parentObject, flattenedList);
            }
            else if (root.Type == JTokenType.Array)
            {
                JArray parentArray = (JArray)root;
                FlattenArray(parentArray, flattenedList);
            }
            else
            {
                string s = root.Path + "." + root.ToString();
                flattenedList.Add(RemoveIndices(s));
            }
        }

        private static HashSet<string> Flatten(JToken root)
        {
            HashSet<string> flattenedList = new HashSet<string>();
            Flatten(root, flattenedList);
            return flattenedList;
        }

        public static bool VerifyPropertiesAbsent(JToken expectedAbsentValue, JToken actualValue, List<string> errorList)
        {
            HashSet<string> expectedAbsentFlattenedList = Flatten(expectedAbsentValue);
            HashSet<string> actualFlattenedList = Flatten(actualValue);

            foreach (string s in expectedAbsentFlattenedList)
            {
                if (actualFlattenedList.Contains(s))
                {
                    string msg = "Unexpected " + s + " in actual.";
                    errorList.Add(msg);
                    Debug.WriteLine(msg);
                    return false;
                }
            }
            return true;
        }
    }
}