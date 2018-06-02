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

namespace DMValidator
{
    static class Constants
    {
        public const string JsonPropertiesRoot = "properties";
        public const string JsonDesiredRoot = "desired";
        public const string JsonReportedRoot = "reported";
        public const string JsonDeviceTwin = "deviceTwin";
        public const string JsonDirectMethodEmptyParams = "{}";
        // public const string JsonClearReportedCmd = "clearReportedCmd";
        public const string JsonClearReportedCmd = "windows.clearReportedPropertiesAsync";

        public const string JsonScenario = "scenario";

        public const string JsonDirectMethodName = "directMethodName";

        public const string TCJsonName = "name";
        public const string TCJsonDescription = "description";
        public const string TCJsonInput = "input";
        public const string TCJsonOutput = "output";
        public const string TCJsonDelay = "delay";
        public const string TCJsonOutputPresent = "present";
        public const string TCJsonOutputAbsent = "absent";

        public const string TCJsonType = "type";
        public const string TCJsonInteractionDeviceTwin = "deviceTwin";
        public const string TCJsonInteractionDirectMethod = "directMethod";
        public const string TCJsonInteractionDotNetApi = "dotNetApi";

        public const string TCJsonMethodReturnJson = "returnJson";
        public const string TCJsonMethodReturnCode = "returnCode";
        public const string TCJsonMethodDeviceTwin = "deviceTwin";

        public const string TCJsonDotNetApiName = "apiName";

        public const string TCJsonSetWindowsUpdateRingAsync = "SetWindowsUpdateRingAsync";

        public const string JsonIgnore = "<ignore>";

        public const string JsonRebootInfo = "rebootInfo";
    }
}