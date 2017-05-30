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
// CSPTests.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "CertificateManagementTest.h"
#include "DeviceHealthAttestationTest.h"
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
    result &= DeviceHealthAttestationTest::RunTest();

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

