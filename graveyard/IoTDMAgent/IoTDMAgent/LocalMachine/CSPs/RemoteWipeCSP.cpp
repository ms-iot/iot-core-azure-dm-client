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

using namespace std;

// Remote Wipe CSP docs
// https://msdn.microsoft.com/en-us/library/windows/hardware/dn904968(v=vs.85).aspx
//

void RemoteWipeCSP::DoWipe()
{
    TRACE(L"RemoteWipeCSP::DoWipe()");
#if 0
    MdmProvision::RunExec(L"./Device/Vendor/MSFT/RemoteWipe/doWipe");
#else
    // Temporary code until we have the actual CSP for RemoteWipe
    const wstring BcdEditCommand = Utils::GetSystemRootFolder() + L"\\system32\\bcdedit.exe /set \"{globalsettings}\" bootflow 0x802";

    unsigned long returnCode = 0;
    std::string output;
    Utils::LaunchProcess(BcdEditCommand, returnCode, output);
    if (0 != returnCode)
    {
        stringstream message;
        message << "Error command: { " << Utils::WideToMultibyte(BcdEditCommand.c_str()) << " } failed with error code: " << hex << returnCode << ", Output:" << output;
        throw DMException(message.str().c_str());
    }

    // Note: Reboot has a 5 minute delay.
    RebootCSP::ExecRebootNow();
#endif
}
