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
#include "..\SharedUtilities\Logger.h"
#include "CSPs\CertificateInfo.h"

#define SERVICE_NAME             L"SystemConfigurator"
#define SERVICE_DISPLAY_NAME     L"System Configurator"
#define SERVICE_START_TYPE       SERVICE_DEMAND_START
#define SERVICE_DEPENDENCIES     L""
#define SERVICE_ACCOUNT          L"NT AUTHORITY\\SYSTEM"
#define SERVICE_PASSWORD         L""

#define SET_TIMEZONE_SWITCH      L"settimezone:"

using namespace std;

int SetTimeZone(const wstring& newValue)
{
    TIME_ZONE_INFORMATION tzi = { 0 };

    tzi.StandardBias = 0;
    tzi.StandardDate.wYear = 2007; tzi.StandardDate.wMonth = 1; tzi.StandardDate.wDay = 1;
    tzi.StandardDate.wHour = 2;    tzi.StandardDate.wMinute = 0; tzi.StandardDate.wSecond = 0;

    tzi.DaylightBias = -60;
    tzi.DaylightDate.wYear = 3000; tzi.DaylightDate.wMonth = 12; tzi.DaylightDate.wDay = 31;
    tzi.DaylightDate.wHour = 2;    tzi.DaylightDate.wMinute = 0; tzi.DaylightDate.wSecond = 0;

    if (0 == _wcsicmp(newValue.c_str(), L"est"))
    {
        tzi.Bias = 300;
        wcsncpy_s(tzi.StandardName, L"Eastern Standard Time", _TRUNCATE);
        wcsncpy_s(tzi.DaylightName, L"Eastern Daylight Time", _TRUNCATE);
    }
    else if (0 == _wcsicmp(newValue.c_str(), L"cst"))
    {
        tzi.Bias = 360;
        wcsncpy_s(tzi.StandardName, L"Central Standard Time", _TRUNCATE);
        wcsncpy_s(tzi.DaylightName, L"Central Daylight Time", _TRUNCATE);
    }
    else if (0 == _wcsicmp(newValue.c_str(), L"mst"))
    {
        tzi.Bias = 420;
        wcsncpy_s(tzi.StandardName, L"Mountain Standard Time", _TRUNCATE);
        wcsncpy_s(tzi.DaylightName, L"Mountain Daylight Time", _TRUNCATE);
    }
    else if (0 == _wcsicmp(newValue.c_str(), L"pst"))
    {
        tzi.Bias = 480;
        wcsncpy_s(tzi.StandardName, L"Pacific Standard Time", _TRUNCATE);
        wcsncpy_s(tzi.DaylightName, L"Pacific Daylight Time", _TRUNCATE);
    }
    else
    {
        TRACE("Unknown time zone...");
        return -1;
    }

    TRACE(tzi.StandardName);

    if (SetTimeZoneInformation(&tzi))
    {
        TRACE("Time zone set successfully.");
    }
    else
    {
        TRACEP("Failed to set time zone. Erro = ", GetLastError());
        return -1;
    }
}

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
        else if (_wcsicmp(L"cleantest", argv[1] + 1) == 0)
        {
            try
            {
                CertificateInfo::DeleteCertificate(L"./Device/Vendor/MSFT/RootCATrustedCertificates/Root", L"6e7127c2d7c3d0aff188db3b386f63ecd98b8935");
            }
            catch (...)
            {
                return -1;
            }
            return 0;
        }
        else if (_wcsnicmp(SET_TIMEZONE_SWITCH, argv[1] + 1, wcslen(SET_TIMEZONE_SWITCH)) == 0)
        {
            TRACE("Setting time zone...");
            wstring newValue = argv[1] + 1 + wcslen(SET_TIMEZONE_SWITCH);
            return SetTimeZone(newValue);
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
