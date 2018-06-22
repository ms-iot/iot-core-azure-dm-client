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

using Newtonsoft.Json;
using Newtonsoft.Json.Linq;

namespace Microsoft.Devices.Management.DMDataContract
{
    public class RemoteWipeDataContract
    {
        public static readonly string NotFound = "<not found>";
        public static readonly string SectionName = "remoteWipe";
        public static readonly string StartRemoteWipeAsync = DMJSonConstants.DTWindowsIoTNameSpace + ".startRemoteWipeAsync";

        public static readonly string JsonResponse = "response";
        public static readonly string JsonScheduled = "scheduled";

        public class WipeParams
        {
            public bool clearTPM;

            public string ToJsonString()
            {
                return JsonConvert.SerializeObject(this);
            }
        }

        public class ReportedProperties
        {
            public string response;

            public static ReportedProperties FromJsonObject(JObject json)
            {
                ReportedProperties reportedProperties = new ReportedProperties();
                reportedProperties.response = Utils.GetString(json, JsonResponse, NotFound);
                return reportedProperties;
            }

            public JObject ToJsonObject()
            {
                return JObject.FromObject(this);
            }
        }

    }
}