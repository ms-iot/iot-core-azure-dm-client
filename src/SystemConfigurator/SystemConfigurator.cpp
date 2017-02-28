#include "stdafx.h"
#include "CommandProcessor.h"
#include "DMService.h"
#include "..\SharedUtilities\Logger.h"
#include "CSPs\EnterpriseModernAppManagementCSP.h"

#define SERVICE_NAME             L"SystemConfigurator"
#define SERVICE_DISPLAY_NAME     L"System Configurator"
#define SERVICE_START_TYPE       SERVICE_DEMAND_START
#define SERVICE_DEPENDENCIES     L""
#define SERVICE_ACCOUNT          L"NT AUTHORITY\\SYSTEM"
#define SERVICE_PASSWORD         L""

using namespace std;

[Platform::MTAThread]
int wmain(int argc, wchar_t *argv[])
{
    TRACE("Entering wmain...");

    if ((argc > 1) && ((*argv[1] == L'-' || (*argv[1] == L'/'))))
    {
        if (_wcsicmp(L"install", argv[1] + 1) == 0)
        {
            DMService::Install(
                SERVICE_NAME,               // Name of service
                SERVICE_DISPLAY_NAME,       // Name to display
                SERVICE_START_TYPE,         // Service start type
                SERVICE_DEPENDENCIES,       // Dependencies
                SERVICE_ACCOUNT,            // Service running account
                SERVICE_PASSWORD            // Password of the account
            );
        }
        else if (_wcsicmp(L"remove", argv[1] + 1) == 0)
        {
            DMService::Uninstall(SERVICE_NAME);
        }
        else if (_wcsicmp(L"debug", argv[1] + 1) == 0)
        {
            Listen();
        }
        else if (_wcsicmp(L"1", argv[1] + 1) == 0)
        {
            vector<wstring> dependentPackages;
            dependentPackages.push_back(L"C:\\temp\\test\\Microsoft.VCLibs.140.00_14.0.24123.0_x64__8wekyb3d8bbwe.appx");
            dependentPackages.push_back(L"C:\\temp\\test\\Microsoft.NET.Native.Runtime.1.4_1.4.24201.0_x64__8wekyb3d8bbwe.appx");
            dependentPackages.push_back(L"C:\\temp\\test\\Microsoft.NET.Native.Framework.1.3_1.3.24201.0_x64__8wekyb3d8bbwe.appx");
            EnterpriseModernAppManagementCSP::InstallApp(L"23983CETAthensQuality.IoTCoreSmartDisplay_1w720vyc4ccym",
                L"C:\\temp\\test\\1.2.9.0\\e37d3b833dea43308bb689999de744c5.appxbundle"
                , dependentPackages);
        }
    }
    else
    {
        TRACE(L"Parameters:");
        TRACE(L" -install  to install the service.");
        TRACE(L" -remove   to remove the service.");
        TRACE(L"");
        TRACE(L"Running service...");

        DMService service(SERVICE_NAME);
        DMService::Run(service);
    }

    TRACE("Exiting wmain.");

    return 0;
}
