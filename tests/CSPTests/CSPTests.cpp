// CSPTests.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "CertificateManagementTest.h"
#include "..\..\src\SharedUtilities\Logger.h"

void ShowUsage()
{
    TRACE("To run this test:");
    TRACE("- Run under a SYSTEM account.");
    TRACE("- SystemConfigurator must not be running (i.e. not local DM is enrolled).");
    TRACE("");
    TRACE("Example:");
    TRACE("    schtasks /Create /SC ONCE /RU \"SYSTEM\" /TN TestCSPsTask /TR C:\\temp\\CSPTests.exe /ST 14:00");
    TRACE("");
    TRACE("Then check for the logs under C:\\Data\\Users\\DefaultAccount\\AppData\\Local\\Temp\\IotDm");
    TRACE("");
    TRACE("On success, the return value is 0.");
    TRACE("");
}

[Platform::MTAThread]
int wmain(int argc, wchar_t *argv[])
{
    ShowUsage();

    bool result = true;

    result &= CertificateManagementTest::RunTest();

    // Add other tests here.

    if (result)
    {
        TRACE("Test Passed!");
    }
    else
    {
        TRACE("Test Failed");
    }
    return result ? 0 : 1;
}

