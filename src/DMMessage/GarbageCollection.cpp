#include "stdafx.h"
#include "..\DMMessage\GarbageCollection.h"
#include "..\DMMessage\GarbageCollectedTempFolder.h"

using namespace Platform;
using namespace concurrency;

namespace Microsoft { namespace Devices { namespace Management { namespace Message
{
    String^ DMGarbageCollection::TempFolder::get()
    {
        return ref new String(SC_CLEANUP_FOLDER); 
    }
}}}}