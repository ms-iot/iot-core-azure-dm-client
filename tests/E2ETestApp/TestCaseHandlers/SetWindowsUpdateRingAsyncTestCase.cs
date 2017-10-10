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

using Mock;
using System;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;
using System.Threading.Tasks;

// For logging...
using Microsoft.Devices.Management;
using Windows.Foundation.Diagnostics;

namespace E2ETestApp
{
    class SetWindowsUpdateRingAsyncTestCase : TestCase
    {
        private const string TCJsonRing = "ring";
        private const string TCJsonSettingsPriority = "settingsPriority";

        public override void Dump()
        {
            Debug.WriteLine("Name                : " + _name);
            Debug.WriteLine("Desription          : " + _description);
            Debug.WriteLine("   Input.           : " + _input.ToString());
            if (_expectedPresentReportedState != null)
            {
                Debug.WriteLine("Expected reported present state : " + _expectedPresentReportedState.ToString());
            }
            if (_expectedAbsentReportedState != null)
            {
                Debug.WriteLine("Expected reported absent state : " + _expectedAbsentReportedState.ToString());
            }
        }

        public static SetWindowsUpdateRingAsyncTestCase FromJson(ILogger logger, JObject testCaseJson)
        {
            logger.Log(LoggingLevel.Information, "        Parsing test case...");

            string name;
            if (!JsonHelpers.TryGetString(testCaseJson, Constants.TCJsonName, out name))
            {
                ReportError(logger, "Missing " + Constants.TCJsonName);
                return null;
            }

            string description;
            if (!JsonHelpers.TryGetString(testCaseJson, Constants.TCJsonDescription, out description))
            {
                ReportError(logger, "Missing " + Constants.TCJsonDescription);
                return null;
            }

            JObject input;
            if (!JsonHelpers.TryGetObject(testCaseJson, Constants.TCJsonInput, out input))
            {
                ReportError(logger, "Missing " + Constants.TCJsonInput);
                return null;
            }

            JObject output;
            JObject expectedPresentReportedState = null;
            JObject expectedAbsentReportedState = null;
            if (!JsonHelpers.TryGetObject(testCaseJson, Constants.TCJsonOutput, out output))
            {
                ReportError(logger, "Missing " + Constants.TCJsonOutput);
                return null;
            }
            else
            {
                JObject deviceTwin;
                if (JsonHelpers.TryGetObject(output, Constants.TCJsonInteractionDeviceTwin, out deviceTwin))
                {
                    if (JsonHelpers.TryGetObject(output, Constants.TCJsonOutputPresent, out expectedPresentReportedState))
                    {
                        expectedPresentReportedState = (JObject)expectedPresentReportedState.DeepClone();
                    }

                    if (JsonHelpers.TryGetObject(output, Constants.TCJsonOutputAbsent, out expectedAbsentReportedState))
                    {
                        expectedAbsentReportedState = (JObject)expectedAbsentReportedState.DeepClone();
                    }
                }
            }

            SetWindowsUpdateRingAsyncTestCase testCase = new SetWindowsUpdateRingAsyncTestCase();
            testCase._name = name;
            testCase._description = description;
            testCase._input = input;
            testCase._expectedPresentReportedState = expectedPresentReportedState;
            testCase._expectedAbsentReportedState = expectedAbsentReportedState;
            return testCase;
        }

        private WindowsUpdateRingState RingStateFromJson(ILogger logger, JObject input)
        {
            string ringString;
            if (!JsonHelpers.TryGetString(input, TCJsonRing, out ringString))
            {
                ReportError(logger, "Missing " + TCJsonRing);
                return null;
            }

            string settingsPriorityString;
            if (!JsonHelpers.TryGetString(input, TCJsonSettingsPriority, out settingsPriorityString))
            {
                ReportError(logger, "Missing " + TCJsonSettingsPriority);
                return null;
            }

            WindowsUpdateRingState state = new WindowsUpdateRingState();
            state.ring = WindowsUpdateRingState.RingFromJsonString(ringString);
            state.settingsPriority = PolicyHelpers.SettingsPriorityFromString(settingsPriorityString);
            return state;
        }

        public override async Task<bool> Execute(ILogger logger, TestParameters testParameters)
        {
            logger.Log(LoggingLevel.Information, "            Executing test case (" + _name + ")...");

            JObject resolvedInput = (JObject)testParameters.ResolveParameters(_input);

            WindowsUpdateRingState state = RingStateFromJson(logger, resolvedInput);

            await _app.DMClient.SetWindowsUpdateRingAsync(state);

            logger.Log(LoggingLevel.Information, "              Analyzing results...");

            Debug.WriteLine("Final Result:");
            Debug.WriteLine("Actual Result: " + _actualReportedState.ToString());

            List<string> errorList = new List<string>();
            bool result = true;

            Debug.WriteLine("---- Expected Present Result:");
            if (_expectedPresentReportedState != null)
            {
                Debug.WriteLine(_expectedPresentReportedState.ToString());
                result &= TestCaseHelpers.VerifyPropertiesPresent("returnValue", _expectedPresentReportedState, _actualReportedState, errorList);
            }

            Debug.WriteLine("---- Expected Absent Result:");
            if (_expectedAbsentReportedState != null)
            {
                Debug.WriteLine(_expectedAbsentReportedState.ToString());
                result &= TestCaseHelpers.VerifyPropertiesAbsent(_expectedAbsentReportedState, _actualReportedState, errorList);
            }

            ReportResult(logger, result, errorList);

            return result;
        }

        private JObject _input;
        private JObject _expectedPresentReportedState;
        private JObject _expectedAbsentReportedState;
    }
}