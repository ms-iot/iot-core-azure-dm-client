#include "stdafx.h"
#include "AppCfg.h"
#include "..\SharedUtilities\Utils.h"
#include "..\SharedUtilities\Logger.h"
#include "..\SharedUtilities\DMException.h"

// strings
#define IotStartup L"C:\\windows\\system32\\iotstartup.exe"
#define StartCmd L" start "
#define StopCmd L" stop "

using namespace std;
using namespace Utils;

void AppCfg::ManageApp(const wstring& appId, bool start)
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

void AppCfg::StartApp(const wstring& appId)
{
    TRACE(__FUNCTION__);
	ManageApp(appId, true);
}

void AppCfg::StopApp(const wstring& appId)
{
	TRACE(__FUNCTION__);
	ManageApp(appId, false);
}
