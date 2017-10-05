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

using System.Collections.Generic;
using Windows.ApplicationModel.Core;
using Windows.Foundation.Diagnostics;
using Windows.UI.Core;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;

namespace E2ETestApp
{
    public interface ILogger
    {
        void Log(LoggingLevel level, string msg);
    }

    public sealed partial class MainPage : Page, ILogger
    {
        public MainPage()
        {
            InitializeComponent();

            EnumerateTestCases();

        }

        public void Log(LoggingLevel level, string msg)
        {
            if (level >= LoggingLevel.Information)
            {
                string fullMsg = level.ToString() + ": " + msg;
                CoreApplication.MainView.CoreWindow.Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
                {
                    StatusList.Items.Insert(0, fullMsg);
                });

                /*
                // ToDo: scrolling doesn't take place correctly.
                StatusList.Items.Add(fullMsg);
                StatusList.SelectedIndex = StatusList.Items.Count - 1;
                StatusList.UpdateLayout();
                StatusList.ScrollIntoView(StatusList.SelectedItem, ScrollIntoViewAlignment.Leading);
                */
            }
            Microsoft.Devices.Management.Logger.Log(msg, LoggingLevel.Verbose);
        }

        private static void FlattenTestTree(TestNode testNode, List<TestNode> targetList)
        {
            targetList.Add(testNode);
            foreach (TestNode child in testNode.Children)
            {
                FlattenTestTree(child, targetList);
            }
        }

        private static IEnumerable<TestNode> FlattenTestTree(TestNode root)
        {
            if (root == null)
            {
                return new List<TestNode>();
            }

            List<TestNode> list = new List<TestNode>();
            FlattenTestTree(root, list);
            return list;
        }

        private async void EnumerateTestCases()
        {
            Log(LoggingLevel.Information, "Enumerating test cases...");

            _testCasesRoot = await TestCaseEnumerator.EnumerateTestCases();
            if (_testCasesRoot == null)
            {
                Log(LoggingLevel.Information, "No test cases found.");
            }
            else
            {
                TestCaseEnumerator.DumpTree(0, _testCasesRoot);
                TestCasesList.ItemsSource = FlattenTestTree(_testCasesRoot);
                Log(LoggingLevel.Information, "Done enumerating test cases.");
            }
        }

        private void OnReset(object sender, RoutedEventArgs e)
        {
            Log(LoggingLevel.Information, "Reset status of selected test cases...");
            _testCasesRoot.Reset();
            Log(LoggingLevel.Information, "Done resetting test cases.");
        }

        private async void ExecuteTestCases()
        {
            Log(LoggingLevel.Information, "Executing selected test cases...");
            await _testCasesRoot.Execute(this, new TestParameters());
            Log(LoggingLevel.Information, "Done executing selected test cases.");
        }

        private void OnExecute(object sender, Windows.UI.Xaml.RoutedEventArgs e)
        {
            ExecuteTestCases();
        }

        TestNode _testCasesRoot;
    }
}
