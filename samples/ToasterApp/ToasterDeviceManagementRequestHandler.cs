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
using System.Threading.Tasks;
using Windows.ApplicationModel;

using Microsoft.Devices.Management;

namespace Toaster
{
    class ToasterDeviceManagementRequestHandler : IDeviceManagementRequestHandler
    {
        MainPage mainPage;

        public ToasterDeviceManagementRequestHandler(MainPage mainPage)
        {
            this.mainPage = mainPage;
        }

        // Answer the question "is it OK to reboot the toaster"
        async Task<bool> IDeviceManagementRequestHandler.IsSystemRebootAllowed()
        {
            bool answer = await this.mainPage.YesNo("Allow reboot?");
            return answer;
        }
    }
}
