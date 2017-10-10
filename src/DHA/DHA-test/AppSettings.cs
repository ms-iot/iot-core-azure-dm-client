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

namespace DHA_test
{
    /// <summary>
    /// This class reads the appsettings.json from the DHA azure functions project and exposes
    /// the settings as key/value pairs.
    /// </summary>
    internal class AppSettings
    {
        public static AppSettings Current = new AppSettings();

        public string GetValue(string keyName)
        {
            return (string)valueJObj.GetValue(keyName);
        }

        private readonly string AppSettingFilename = @"..\..\..\DHA\appsettings.json";

        private AppSettings()
        {
            var json = System.IO.File.ReadAllText(AppSettingFilename);
            jobj = JObject.Parse(json);
            valueJObj = (JObject)jobj.GetValue("Values");
        }

        private JObject jobj;
        private JObject valueJObj;
    }
}
