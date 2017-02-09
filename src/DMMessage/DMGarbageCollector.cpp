#include "stdafx.h"
#include "..\DMMessage\DMGarbageCollector.h"
#include "..\DMMessage\DMGarbageCollectorTempFolder.h"

using namespace Platform;
using namespace concurrency;

namespace Microsoft { namespace Devices { namespace Management { namespace Message
{
    String^ DMGarbageCollector::TempFolder::get()
    {
        return ref new String(SC_CLEANUP_FOLDER); 
    }
}}}}