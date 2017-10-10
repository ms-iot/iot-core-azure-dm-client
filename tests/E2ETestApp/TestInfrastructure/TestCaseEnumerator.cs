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
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;
using System.Threading.Tasks;
using Windows.ApplicationModel;
using Windows.Storage;

namespace E2ETestApp
{
    class TestCaseEnumerator
    {
        private const string TestCasesRootFolder = "TestCases";
        private const string TestCasesRootName = "Root";
        private const string TestCasesRootDescription = "";

        private static async Task EnumerateTestCasesAsync(uint level, StorageFolder parentFolder, TestNode parentNode)
        {
            if (parentFolder == null || parentNode == null)
            {
                return;
            }

            // Process child folders...
            IReadOnlyList<StorageFolder> childFolders = await parentFolder.GetFoldersAsync();
            foreach (StorageFolder childFolder in childFolders)
            {
                // Create a new test node...
                TestNode childNode = new TestNode(parentNode, level + 1, childFolder.DisplayName, childFolder.Path);
                parentNode.AddChild(childNode);

                // Recurse...
                await EnumerateTestCasesAsync(level + 1, childFolder, childNode);
            }

            // Process child files...
            IReadOnlyList<StorageFile> childFiles = await parentFolder.GetFilesAsync();
            foreach (StorageFile childFile in childFiles)
            {
                TestNode childNode = new TestNode(parentNode, level + 1, childFile.Name, childFile.Path);
                parentNode.AddChild(childNode);
            }
        }

        public static async Task<TestNode> EnumerateTestCasesAsync()
        {
            var folder = Package.Current.InstalledLocation;
            StorageFolder rootFolder = await folder.GetFolderAsync(TestCasesRootFolder);
            if (rootFolder == null)
            {
                return null;
            }

            TestNode rootNode = new TestNode(null /*parent*/, 0 /*level*/, TestCasesRootName, TestCasesRootDescription);
            await EnumerateTestCasesAsync(0 /*level*/, rootFolder, rootNode);
            return rootNode;
        }

        public static void DumpTree(uint level, TestNode root)
        {
            if (root == null)
            {
                return;
            }

            // Create indentation string...
            StringBuilder sb = new StringBuilder();
            for (uint i = 0; i < level; ++i)
            {
                sb.Append("    ");
            }
            string indent = sb.ToString();

            // Dump the content...
            Debug.WriteLine(indent + "[" + root.Name + "]");
            Debug.WriteLine(indent + "    Path    = " + root.Path);

            // Recurse...
            foreach (TestNode tn in root.Children)
            {
                DumpTree(level + 1, tn);
            }
        }
    }
}
