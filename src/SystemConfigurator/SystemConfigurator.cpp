#include "stdafx.h"
#include "CommandProcessor.h"
#include "DMService.h"
#include "..\SharedUtilities\Logger.h"

#define SERVICE_NAME             L"SystemConfigurator"
#define SERVICE_DISPLAY_NAME     L"System Configurator"
#define SERVICE_START_TYPE       SERVICE_DEMAND_START
#define SERVICE_DEPENDENCIES     L""
#define SERVICE_ACCOUNT          L"NT AUTHORITY\\SYSTEM"
#define SERVICE_PASSWORD         L""

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
