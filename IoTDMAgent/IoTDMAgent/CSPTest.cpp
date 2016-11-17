#include "stdafx.h"
#include <windows.h>
#include "IoTDMService.h"
#include "LocalMachine\CSPs\EnterpriseModernAppManagementCSP.h"


int main(Platform::Array<Platform::String^>^ args)
{
    int action = 3;
    if (action == 1)
    {
        auto ver = EnterpriseModernAppManagementCSP::GetInstalledApps();
    }
    else if (action == 2)
    {
        std::vector<std::wstring> dependencies = {
            LR"(C:\projects\AppXSample\AppPackages\AppXSample_1.0.0.0_Debug_Test\Dependencies\x86\Microsoft.NET.CoreRuntime.1.0.appx)",
            LR"(C:\projects\AppXSample\AppPackages\AppXSample_1.0.0.0_Debug_Test\Dependencies\x86\Microsoft.VCLibs.x86.Debug.14.00.appx)"
        };

        auto packageUri = LR"(C:\projects\AppXSample\AppPackages\AppXSample_1.0.0.0_Debug_Test\AppXSample_1.0.0.0_x86_x64_arm_Debug.appxbundle)";

        EnterpriseModernAppManagementCSP::InstallApp(L"0de8badf-807f-4641-9cb9-95e4f9c34fef_9nthh9tntkkay", packageUri, dependencies);
    }
    else if (action == 3)
    {
        EnterpriseModernAppManagementCSP::UninstallApp(L"0de8badf-807f-4641-9cb9-95e4f9c34fef_9nthh9tntkkay");
    }

    return 0;
}