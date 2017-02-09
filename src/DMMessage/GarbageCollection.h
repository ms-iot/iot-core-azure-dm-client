#pragma once

using namespace Platform;

namespace Microsoft { namespace Devices { namespace Management { namespace Message
{
    public ref class DMGarbageCollection sealed {
    public:
        static property String^ TempFolder { String^ get(); }
    };
}}}}