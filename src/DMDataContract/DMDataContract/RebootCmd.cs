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

namespace Microsoft.Devices.Management.DMDataContract
{
    public class RebootCmdDataContract
    {
        public static readonly string NotFound = "<not found>";
        public static readonly string SectionName = "rebootCmd";
        public static readonly string RebootCmdAsync = DMJSonConstants.DTWindowsIoTNameSpace + ".rebootAsync";

        public static readonly string JsonResponse = "response";

        public static readonly string JsonScheduled = "scheduled";
        public static readonly string JsonAllowed = "allowed";
        public static readonly string JsonDisabled = "disabled";
        public static readonly string JsonInActiveHours = "inActiveHours";
        public static readonly string JsonRejectedByApp = "rejectedByApp";

        public enum ResponseValue
        {
            Allowed,
            Scheduled,
            Disabled,
            InActiveHours,
            RejectedByApp
        }

        public RebootCmdDataContract()
        {
        }

        public RebootCmdDataContract(ResponseValue r)
        {
            response = ToString(r);
        }

        public static string ToString(ResponseValue status)
        {
            switch (status)
            {
                case ResponseValue.Allowed: return JsonAllowed;
                case ResponseValue.Scheduled: return JsonScheduled;
                case ResponseValue.Disabled: return JsonDisabled;
                case ResponseValue.InActiveHours: return JsonInActiveHours;
                case ResponseValue.RejectedByApp: return JsonRejectedByApp;
            }
            return status.ToString();
        }

        public class ReportedProperties
        {
            public string response;

            public void LoadFrom(JObject json)
            {
                response = Utils.GetString(json, JsonResponse, NotFound);
            }
        }

        public string response;
    }

}


