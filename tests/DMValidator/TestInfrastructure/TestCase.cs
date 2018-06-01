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

using System.Windows;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading.Tasks;

namespace DMValidator
{
    abstract class TestCase
    {
        public string Name
        {
            get
            {
                return _name;
            }
        }

        public virtual void Dump()
        {
        }

        protected static void ReportError(ILogger logger, string msg)
        {
            logger.Log(LogLevel.Error, msg);
        }

        protected void ReportResult(ILogger logger, bool result, IEnumerable<string> errorList)
        {
            if (result)
            {
                logger.Log(LogLevel.Information, "Test " + _name + " succeeded!");
            }
            else
            {
                logger.Log(LogLevel.Information, "Test " + _name + " Failed!");
            }
        }

        public abstract Task<bool> Execute(ILogger logger, IoTHubManager client, TestParameters testParameters);

        // Common properties...
        protected string _name;
        protected string _description;
    }
}