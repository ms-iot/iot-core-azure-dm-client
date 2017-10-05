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
using System.Threading.Tasks;
using Windows.Foundation.Diagnostics;

using Mock.Microsoft.Azure.Devices.Client;
using Mock.App;

namespace E2ETestApp
{
    class TestScenario
    {
        private const string JsonScenario = "scenario";
        private const string DeviceId = "MockDevice";
        private const string EmptyTwin = "{   \"deviceId\": \"gmileka09\"," +
                                             "\"etag\": \"AAAAAAAAAAE=\"," +
                                              "\"properties\": {" +
                                                "\"desired\": {" +
                                                  "\"$metadata\": {" +
                                                    "\"$lastUpdated\": \"2017-09-19T23:23:45.1657119Z\"" +
                                                  "}," +
                                                  "\"$version\": 1" +
                                                "}," +
                                                "\"reported\": {" +
                                                  "\"$metadata\": {" +
                                                    "\"$lastUpdated\": \"2017-09-19T23:23:45.1657119Z\"" +
                                                  "}," +
                                                  "\"$version\": 1" +
                                                "}" +
                                              "}" +
                                           "}";

        public TestScenario()
        {
            _testCases = new List<TestCase>();
        }

        private static TestCase TestCaseFromJson(ILogger logger, JObject testCaseJson)
        {
            string type;
            if (!JsonHelpers.TryGetString(testCaseJson, Constants.TCJsonType, out type))
            {
                logger.Log(LoggingLevel.Error, "        Missing " + Constants.TCJsonType + " attribute.");
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
                                    testCase = SetWindowsUpdateRingAsyncTestCase.FromJson(logger, testCaseJson);
                                    break;
                            }

                        }
                    }
                    break;
            }

            return testCase;
        }

        private async Task InitializeMockObjectsAsync(ILogger logger)
        {
            logger.Log(LoggingLevel.Information, "          Initializing mock objects...");

            // Create the twin...
            _twin = new Twin(DeviceId, EmptyTwin);

            // Create the device client...
            _app = new DeviceClientApp();
            await _app.InitializeDeviceClientAsync(_twin);

            // Create the portal client...
            _deviceController = new RegistryManager(_twin);
        }

        public static TestScenario FromJson(ILogger logger, string scenarioFileName, object deserializedContent)
        {
            logger.Log(LoggingLevel.Information, "    Parsing scenario (" + scenarioFileName + ")...");

            if (deserializedContent == null || !(deserializedContent is JObject))
            {
                logger.Log(LoggingLevel.Error, "(" + scenarioFileName + ") is not a JObject. Skipping...");
                return null;
            }

            JArray testCases;
            TestScenario testScenario = null;
            if (JsonHelpers.TryGetArray((JObject)deserializedContent, JsonScenario, out testCases))
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

                        logger.Log(LoggingLevel.Information, "          Adding (" + testCase.Name + ") to scenario.");
                        testScenario._testCases.Add(testCase);
                    }
                }
            }

            if (testScenario == null)
            {
                logger.Log(LoggingLevel.Information, "(" + scenarioFileName + ") has no test cases defined. Skipping...");
            }

            return testScenario;
        }

        public async Task<bool> Execute(ILogger logger, TestParameters testParameters)
        {
            logger.Log(LoggingLevel.Information, "          Executing test scenario (" + _scenarioFileName + ").");

            bool result = true;

            await InitializeMockObjectsAsync(logger);

            foreach (TestCase testCase in _testCases)
            {
                testCase.Initialize(_twin, _app, _deviceController);
                result = await testCase.Execute(logger, testParameters);
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

        // Mock objects
        private Twin _twin;
        private DeviceClientApp _app;
        private RegistryManager _deviceController;
    }
}