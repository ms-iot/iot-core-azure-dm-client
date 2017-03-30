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
#include "AppCfg.h"
#include "..\SharedUtilities\Utils.h"
#include "..\SharedUtilities\Logger.h"
#include "..\SharedUtilities\DMException.h"

// strings
#define IotStartup L"C:\\windows\\system32\\iotstartup.exe"
#define StartCmd L" run "
#define StopCmd L" stop "

using namespace std;
using namespace Utils;

void AppCfg::StartStopApp(const wstring& appId, bool start)
{
    TRACE(__FUNCTION__);

    unsigned long returnCode;
    string output;
    wstring cmd = IotStartup;
    if (start) { cmd += StartCmd; }
    else       { cmd += StopCmd; }
    cmd += appId;
    Utils::LaunchProcess(cmd, returnCode, output);
    if (returnCode != 0)
    {
        throw DMExceptionWithErrorCode("Error: iotstartup.exe returned an error code.", returnCode);
    }
}
