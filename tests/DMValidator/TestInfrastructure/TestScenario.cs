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

using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System.Collections.Generic;
using System.IO;
using System.Threading.Tasks;

namespace DMValidator
{
    class TestScenario
    {
        private TestScenario()
        {
            _testCases = new List<TestCase>();
        }

        public static async Task<bool> Run(ILogger logger, string scenarioFileName, TestParameters testParameters)
        {
            logger.Log(LogLevel.Information, "Reading test scenario: " + scenarioFileName);

            string jsonString = File.ReadAllText(scenarioFileName);
            object deserializedObject = JsonConvert.DeserializeObject(jsonString);

            logger.Log(LogLevel.Verbose, "Definition:\n" + jsonString);

            TestScenario ts = FromJson(logger, scenarioFileName, deserializedObject);

            return await ts.Execute(logger, testParameters);
        }

        private static TestCase TestCaseFromJson(ILogger logger, JObject testCaseJson)
        {
            string type;
            if (!JsonHelpers.TryGetString(testCaseJson, Constants.TCJsonType, out type))
            {
                return null;
            }

            TestCase testCase = null;
            switch (type)
            {
                case Constants.TCJsonInteractionDeviceTwin:
                    testCase = DeviceTwinTestCase.FromJson(logger, testCaseJson);
                    break;
                case Constants.TCJsonInteractionDirectMethod:
                    testCase = DirectMethodTestCase.FromJson(logger, testCaseJson);
                    break;
                case Constants.TCJsonInteractionDotNetApi:
                    {
                        string dotNetApiName;
                        if (JsonHelpers.TryGetString(testCaseJson, Constants.TCJsonDotNetApiName, out dotNetApiName))
                        {
                            switch (dotNetApiName)
                            {
                                case Constants.TCJsonSetWindowsUpdateRingAsync:
                                    // testCase = SetWindowsUpdateRingAsyncTestCase.FromJson(testCaseJson);
                                    break;
                            }

                        }
                    }
                    break;
            }

            return testCase;
        }

        private static TestScenario FromJson(ILogger logger, string scenarioFileName, object deserializedContent)
        {
            if (deserializedContent == null || !(deserializedContent is JObject))
            {
                return null;
            }

            JArray testCases;
            TestScenario testScenario = null;
            if (JsonHelpers.TryGetArray((JObject)deserializedContent, Constants.JsonScenario, out testCases))
            {
                foreach (JToken testCaseToken in testCases)
                {
                    if (!(testCaseToken is JObject))
                    {
                        continue;
                    }
                    TestCase testCase = TestCaseFromJson(logger, (JObject)testCaseToken);
                    if (testCase != null)
                    {
                        if (testScenario == null)
                        {
                            testScenario = new TestScenario();
                            testScenario._scenarioFileName = scenarioFileName;
                        }

                        testScenario._testCases.Add(testCase);
                    }
                }
            }

            if (testScenario == null)
            {
                logger.Log(LogLevel.Warning, "Warning: (" + scenarioFileName + ") has no test cases defined. Skipping...");
            }

            return testScenario;
        }

        private async Task<bool> ClearDeviceTwin(ILogger logger, IoTHubManager client, TestParameters testParameters)
        {
            logger.Log(LogLevel.Information, "Clearing device: " + testParameters.IoTHubDeviceId);

            await client.UpdateDesiredProperties(testParameters.IoTHubDeviceId, Constants.JsonRebootInfo, null);
            DeviceMethodReturnValue ret = await client.InvokeDirectMethod(testParameters.IoTHubDeviceId, Constants.JsonClearReportedCmd, Constants.JsonDirectMethodEmptyParams);
            if (ret.Status != IoTHubManager.DirectMethodSuccessCode)
            {
                logger.Log(LogLevel.Error, "Failed to clear the reported properties. Code: " + ret.Status);
                return false;
            }

            logger.Log(LogLevel.Information, "Cleared the device twin successfully");
            return true;
        }

        private async Task<bool> Execute(ILogger logger, TestParameters testParameters)
        {
            logger.Log(LogLevel.Information, "          Executing test scenario (" + _scenarioFileName + ").");

            IoTHubManager client = new IoTHubManager(testParameters.IoTHubConnectionString);

            bool result = await ClearDeviceTwin(logger, client, testParameters);
            if (!result)
            {
                return result;
            }

            foreach (TestCase testCase in _testCases)
            {
                result = await testCase.Execute(logger, client, testParameters);
                if (!result)
                {
                    break;
                }
            }

            return result;
        }

        // Data members...
        private string _scenarioFileName;
        private List<TestCase> _testCases;
    }
}