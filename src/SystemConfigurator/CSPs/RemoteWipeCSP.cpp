#include "stdafx.h"
#include "RemoteWipeCSP.h"
#include "MdmProvision.h"
#include "RebootCSP.h"
#include "..\SharedUtilities\Logger.h"
#include "..\SharedUtilities\DMException.h"

using namespace std;

// Remote Wipe CSP docs
// https://msdn.microsoft.com/en-us/library/windows/hardware/dn904968(v=vs.85).aspx
//

void RemoteWipeCSP::DoWipe()
{
    TRACE(__FUNCTION__);
    MdmProvision::RunExec(L"./Device/Vendor/MSFT/RemoteWipe/doWipe");
}
