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
