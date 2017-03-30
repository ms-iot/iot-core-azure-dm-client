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
