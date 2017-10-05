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

using System.Threading.Tasks;

namespace Mock.Microsoft.Azure.Devices.Client
{
    public delegate Task ReportedPropertyUpdateCallback(TwinCollection reportedProperties);

    class RegistryManager
    {
        public RegistryManager(Twin twin)
        {
            _twin = twin;
            _twin.SetReportedPropertyUpdateCallbackAsync(OnReportedPropertyUpdated);
        }

        public async Task UpdateDesired(string updateJson)
        {
            await _twin.UpdateDesired(updateJson);
        }

        public async Task<MethodResponse> InvokeDirectMethod(string name, string parameters)
        {
            MethodResponse result = await _twin.InvokeDirectMethod(name, parameters);
            return result;
        }

        private async Task OnReportedPropertyUpdated(TwinCollection reportedProperties)
        {
            if (_reportedPropertyUpdateCallback != null)
            {
                await _reportedPropertyUpdateCallback(reportedProperties);
            }
        }

        public void SetReportedPropertyUpdateCallbackAsync(ReportedPropertyUpdateCallback callback)
        {
            _reportedPropertyUpdateCallback = callback;
        }

        Twin _twin;
        ReportedPropertyUpdateCallback _reportedPropertyUpdateCallback;
    }
}