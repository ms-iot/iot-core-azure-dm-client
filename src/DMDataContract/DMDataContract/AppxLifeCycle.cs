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
    public class AppxLifeCycleDataContract
    {
        public const string NotFound = "<not found>";
        public const string SectionName = "manageAppLifeCycleCmd";
        public const string ManageAppLifeCycleAsync = DMJSonConstants.DTWindowsIoTNameSpace + ".manageAppLifeCycleAsync";

        public const string JsonPkgFamilyName = "pkgFamilyName";
        public const string JsonAction = "action";
        public const string JsonStart = "start";
        public const string JsonStop = "stop";

        public class ManageAppLifeCycleParams
        {
            public string pkgFamilyName;
            public string action;

            public void LoadFrom(JObject jsonObject)
            {
                pkgFamilyName = Utils.GetString(jsonObject, JsonPkgFamilyName, NotFound);
                action = Utils.GetString(jsonObject, JsonAction, NotFound);
            }

            public void LoadFrom(string jsonString)
            {
                LoadFrom((JObject)JsonConvert.DeserializeObject(jsonString));
            }
        }

    }

}


