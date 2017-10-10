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

namespace Microsoft.Devices.Management.DMDataContract
{
    public class CommonDataContract
    {
        public const string NotFound = "<not found>";
        public const string ReportAllAsync = DMJSonConstants.DTWindowsIoTNameSpace + ".reportAllAsync";
        public const string JsonQuery = "?";
        public const string JsonRefreshing = "refreshing";
        public const string JsonPending = "pending";
        public const string JsonNoString = "no";
        public const string JsonYesString = "yes";
        public const string JsonReportProperties = "reportProperties";
        public const string JsonApplyProperties = "applyProperties";

        public static bool BooleanFromYesNoJsonString(string s)
        {
            switch (s)
            {
                case JsonYesString: return true;
                case JsonNoString: return false;
            }
            throw new Exception("Unknown Yes/No value: " + s);
        }

        public static string BooleanToYesNoJsonString(bool b)
        {
            if (b)
                return JsonYesString;
            return JsonNoString;
        }
    }
}