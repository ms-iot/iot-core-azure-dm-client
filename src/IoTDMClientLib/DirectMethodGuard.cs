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
using System.Diagnostics;
using System.Threading.Tasks;

namespace Microsoft.Devices.Management
{
    // Wrapper class for direct method calls.  This class catches exceptions thrown by "function",
    // logs diagnostics and return a generic error response with the appropriate exception message.
    internal class DirectMethodGuard
    {
        public DirectMethodGuard(string methodName, Func<string, Task<string>> function)
        {
            this._methodName = methodName;
            this._method = function;
        }

        public async Task<string> Invoke(string arg)
        {
            Debug.WriteLine($"Executing direct method: {this._methodName}");

            try
            {
                return await _method(arg);
            }
            catch (Exception e)
            {
                var response = new { response = "rejected:", reason = e.Message };
                return JsonConvert.SerializeObject(response);
            }
        }

        private string _methodName;
        private Func<string, Task<string>> _method;
    }
}
