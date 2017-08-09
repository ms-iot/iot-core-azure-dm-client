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
#pragma once

using namespace Platform;
using namespace Platform::Metadata;
using namespace Windows::Data::Json;

namespace Microsoft { namespace Devices { namespace Management { namespace Message
{
    public enum class ResponseStatus
    {
        Success,
        Failure
    };

    public enum class ErrorSubSystem
    {
        DeviceManagement = 0x0000,
        Win32            = 0x0001,
        Unknown          = 0xFFFF
    };

    class ErrorSubSystemConverter
    {
    public:
        static ErrorSubSystem FromDouble(double subSystem)
        {
            ErrorSubSystem value = ErrorSubSystem::Unknown;

            switch (static_cast<int>(subSystem))
            {
            case 0:
                value = ErrorSubSystem::DeviceManagement;
                break;
            case 1:
                value = ErrorSubSystem::Win32;
                break;
            }

            return value;
        }
    };

    public enum class DeviceManagementErrors
    {
        SetTimeZoneGenericError = 0x00000001,
    };

}}}}
