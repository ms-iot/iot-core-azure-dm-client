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
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Threading.Tasks;
using Windows.Foundation.Diagnostics;
using Windows.Storage;

namespace E2ETestApp
{
    enum TestNodeStatus
    {
        NotStarted,
        Started,
        Succeeded,
        Failed
    }

    class TestNode : INotifyPropertyChanged
    {
        private const string PropertyActive = "Active";
        private const string PropertyStatus = "Status";

        public event PropertyChangedEventHandler PropertyChanged;

        public string Name { get; private set; }
        public uint Level { get; private set; }
        public string Path { get; private set; }
        public bool HasChildren { get; private set; }

        public bool Active
        {
            get
            {
                return _active;
            }
            set
            {
                if (value != _active)
                {
                    _active = value;

                    TestNode parent = Parent;
                    while (parent != null)
                    {
                        parent.OnChildActiveStateChanged();
                        parent = parent.Parent;
                    }

                    foreach (TestNode child in Children)
                    {
                        child.SetActiveState(_active);
                    }

                    RaisePropertyChanged(PropertyActive);
                }

            }
        }

        public TestNodeStatus Status
        {
            get
            {
                return _status;
            }
            set
            {
                if (value != _status)
                {
                    _status = value;

                    TestNode parent = Parent;
                    while (parent != null)
                    {
                        parent.OnChildStatusChanged();
                        parent = parent.Parent;
                    }

                    // Only reset propagates to children...
                    if (_status == TestNodeStatus.NotStarted)
                    {
                        foreach (TestNode child in Children)
                        {
                            child.SetStatus(_status);
                        }
                    }

                    RaisePropertyChanged(PropertyStatus);
                }
            }
        }

        public TestNode Parent { get; set; }
        public IEnumerable<TestNode> Children
        {
            get
            {
                return _children;
            }
        }

        public TestNode(TestNode parent, uint level, string name, string path)
        {
            this.Name = name;
            this.Path = path;
            this._active = true;
            this.Level = level;
            this.Parent = parent;
            this.HasChildren = false;
            this._children = new List<TestNode>();
        }

        private void RaisePropertyChanged(string name)
        {
            if (PropertyChanged != null)
            {
                PropertyChanged(this, new PropertyChangedEventArgs(name));
            }
        }

        private void SetActiveState(bool active)
        {
            if (_active != active)
            {
                _active = active;
                RaisePropertyChanged(PropertyActive);
            }
            foreach (TestNode child in Children)
            {
                child.SetActiveState(_active);
            }
        }

        private void SetStatus(TestNodeStatus status)
        {
            if (_status != status)
            {
                _status = status;
                RaisePropertyChanged(PropertyStatus);
            }
            foreach (TestNode child in Children)
            {
                child.SetStatus(_status);
            }
        }

        private void OnChildActiveStateChanged()
        {
            bool oneChildOn = false;
            foreach (TestNode child in Children)
            {
                if (child.Active)
                {
                    oneChildOn = true;
                    break;
                }
            }
            if (_active != oneChildOn)
            {
                _active = oneChildOn;
                RaisePropertyChanged(PropertyActive);
            }
        }

        private void OnChildStatusChanged()
        {
            bool oneChildStarted = false;
            bool oneChildSucceeded = false;
            bool oneChildFailed = false;

            foreach (TestNode child in Children)
            {
                if (child.Status == TestNodeStatus.Started)
                {
                    oneChildStarted = true;
                }
                if (child.Status == TestNodeStatus.Succeeded)
                {
                    oneChildSucceeded = true;
                }
                if (child.Status == TestNodeStatus.Failed)
                {
                    oneChildFailed = true;
                }
            }

            if (oneChildStarted)
            {
                // Some are still running...
                if (_status != TestNodeStatus.Started)
                {
                    _status = TestNodeStatus.Started;
                    RaisePropertyChanged(PropertyStatus);
                }
            }
            else if (oneChildFailed)
            {
                // Nothing is running, and some have failed...
                if (_status != TestNodeStatus.Failed)
                {
                    _status = TestNodeStatus.Failed;
                    RaisePropertyChanged(PropertyStatus);
                }
            }
            else if (oneChildSucceeded)
            {
                // Nothing is running, nothing has failed, and some succeeded...
                if (_status != TestNodeStatus.Succeeded)
                {
                    _status = TestNodeStatus.Succeeded;
                    RaisePropertyChanged(PropertyStatus);
                }
            }
            else
            {
                // Nothing is running, failed, or succeeded -> not started...
                if (_status != TestNodeStatus.NotStarted)
                {
                    _status = TestNodeStatus.NotStarted;
                    RaisePropertyChanged(PropertyStatus);
                }
            }
        }

        public void AddChild(TestNode testNode)
        {
            _children.Add(testNode);
            HasChildren = true;
        }

        public void Reset()
        {
            if (HasChildren)
            {
                foreach (TestNode childNode in Children)
                {
                    childNode.Reset();
                }
            }
            else
            {
                Status = TestNodeStatus.NotStarted;
            }
        }

        // For each (selected) node, load test case file, and execute it...
        public async Task Execute(ILogger logger, TestParameters testParameters)
        {
            if (!Active)
            {
                logger.Log(LoggingLevel.Information, "Test node (" + Name + ") is not active. Skipping...");
                return;
            }

            Status = TestNodeStatus.Started;

            if (!HasChildren)
            {
                // If a leaf node...
                logger.Log(LoggingLevel.Information, "  Reading (" + Name + ") test node file: " + Path);

                // Read the file contents...
                StorageFile file = await StorageFile.GetFileFromPathAsync(Path);
                string jsonString = await FileIO.ReadTextAsync(file);
                object deserializedObject = JsonConvert.DeserializeObject(jsonString);

                // Parse the contents...
                TestScenario scenario = TestScenario.FromJson(logger, Name, deserializedObject);
                if (scenario != null)
                {
                    // Execute...
                    bool result = await scenario.Execute(logger, testParameters);

                    // Update status...
                    Status = result ? TestNodeStatus.Succeeded : TestNodeStatus.Failed;
                }
            }
            else
            {
                foreach (TestNode childNode in Children)
                {
                    await childNode.Execute(logger, testParameters);
                }
            }
        }

        private List<TestNode> _children;
        private bool _active;
        private TestNodeStatus _status;
    }

}
