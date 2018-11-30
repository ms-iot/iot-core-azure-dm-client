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
#include "CommandProcessor.h"
#include "DMService.h"
#include "Utils.h"
#include "..\SharedUtilities\Logger.h"
#ifdef _DEBUG
#include "..\SharedUtilities\Impersonator.h"
#endif // _DEBUG

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
    //
    // HKLM\\Software\\Microsoft\\IoTDM
    //   DebugLogFile
    //
    wstring logFileName;
    if (ERROR_SUCCESS == Utils::TryReadRegistryValue(IoTDMRegistryRoot, RegDebugLogFile, logFileName))
    {
        gLogger.SetLogFileName(logFileName);
    }

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
#ifdef _DEBUG
        else if (_wcsicmp(L"debug", argv[1] + 1) == 0)
        {
            Listen();
        }
        else if (_wcsicmp(L"dmuserinfo", argv[1] + 1) == 0)
        {
            auto name = Utils::GetDmUserName();
            printf("user name: %S\r\n", name.c_str());
            auto sid = Utils::GetDmUserSid();
            printf("user  sid: %S\r\n", sid.c_str());
            auto temp = Utils::GetDmTempFolder();
            printf("temp folder: %S\r\n", temp.c_str());
            auto user = Utils::GetDmUserFolder();
            printf("user folder: %S\r\n", user.c_str());
        }
        else if (_wcsicmp(L"testImpersonateShellHost", argv[1] + 1) == 0)
        {
            Impersonator impersonator;
            impersonator.ImpersonateShellHost();
        }
#endif // _DEBUG
    }
    else
    {
        TRACE(L"Parameters:");
        TRACE(L" -install  to install the service.");
        TRACE(L" -remove   to remove the service.");
        TRACE(L"");
        TRACE(L"Running service...");

        auto userFolder = Utils::GetDmUserFolder();
        Utils::EnsureFolderExists(userFolder);

        DMService service(SERVICE_NAME);
        DMService::Run(service);
    }

    TRACE("Exiting wmain.");

    return 0;
}
