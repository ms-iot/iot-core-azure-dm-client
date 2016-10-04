#include "stdafx.h"
#include "ServiceBase.h"
#include "Utilities\Utils.h"
#include "Utilities\Logger.h"
#include <assert.h>
#include <strsafe.h>

using namespace std;

CServiceBase *CServiceBase::s_service = NULL;

void CServiceBase::Run(CServiceBase &service)
{
    TRACE("CServiceBase::Run()");

    s_service = &service;

    SERVICE_TABLE_ENTRY serviceTable[] = 
    {
        { const_cast<LPWSTR>(service._name.c_str()), ServiceMain },
        { NULL, NULL }
    };

    if (!StartServiceCtrlDispatcher(serviceTable))
    {
        throw exception();
    }
}

void WINAPI CServiceBase::ServiceMain(DWORD argc, PWSTR *argv)
{
    TRACE("CServiceBase::ServiceMain()");
    assert(s_service != NULL);

    s_service->_statusHandle = RegisterServiceCtrlHandler(s_service->_name.c_str(), ServiceCtrlHandler);
    if (s_service->_statusHandle == NULL)
    {
        throw GetLastError();
    }

    s_service->Start(argc, argv);
}

void WINAPI CServiceBase::ServiceCtrlHandler(DWORD ctrl)
{
    TRACE("CServiceBase::ServiceCtrlHandler()");

    switch (ctrl)
    {
        case SERVICE_CONTROL_STOP:
            s_service->Stop();
            break;
        case SERVICE_CONTROL_PAUSE:
            s_service->Pause();
            break;
        case SERVICE_CONTROL_CONTINUE:
            s_service->Continue();
            break;
        case SERVICE_CONTROL_SHUTDOWN:
            s_service->Shutdown();
            break;
        case SERVICE_CONTROL_INTERROGATE:
            break;
        default: break;
    }
}

CServiceBase::CServiceBase(const std::wstring& serviceName)
{
    TRACE("CServiceBase.ctor()");
    assert(serviceName.size() != 0);

    _name = serviceName;
    _statusHandle = NULL;

    _status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    _status.dwCurrentState = SERVICE_START_PENDING;

    DWORD dwControlsAccepted = 0;
    dwControlsAccepted |= SERVICE_ACCEPT_STOP;
    dwControlsAccepted |= SERVICE_ACCEPT_SHUTDOWN;
    dwControlsAccepted |= SERVICE_ACCEPT_PAUSE_CONTINUE;
    _status.dwControlsAccepted = dwControlsAccepted;

    _status.dwWin32ExitCode = NO_ERROR;
    _status.dwServiceSpecificExitCode = 0;
    _status.dwCheckPoint = 0;
    _status.dwWaitHint = 0;
}

void CServiceBase::Start(DWORD argc, LPWSTR *lpszArgv)
{
    TRACE("CServiceBase::Start()");
    try
    {
        SetServiceStatus(SERVICE_START_PENDING);
        OnStart(argc, lpszArgv);
        SetServiceStatus(SERVICE_RUNNING);
    }
    catch (DWORD errorCode)
    {
        WriteErrorLogEntry(L"Service Start", errorCode);
        SetServiceStatus(SERVICE_STOPPED, errorCode);
    }
    catch (...)
    {
        WriteEventLogEntry(L"Service failed to start.", EVENTLOG_ERROR_TYPE);
        SetServiceStatus(SERVICE_STOPPED);
    }
}

void CServiceBase::OnStart(DWORD argc, LPWSTR *lpszArgv)
{
    TRACE("CServiceBase::OnStart()");
}

void CServiceBase::Stop()
{
    TRACE("CServiceBase::Stop()");

    DWORD originalState = _status.dwCurrentState;
    try
    {
        SetServiceStatus(SERVICE_STOP_PENDING);
        OnStop();
        SetServiceStatus(SERVICE_STOPPED);
    }
    catch (DWORD errorCode)
    {
        WriteErrorLogEntry(L"Service Stop", errorCode);
        SetServiceStatus(originalState);
    }
    catch (...)
    {
        WriteEventLogEntry(L"Service failed to stop.", EVENTLOG_ERROR_TYPE);
        SetServiceStatus(originalState);
    }
}

void CServiceBase::OnStop()
{
    TRACE("CServiceBase::OnStop()");
}

void CServiceBase::Pause()
{
    TRACE("CServiceBase::Pause()");

    try
    {
        SetServiceStatus(SERVICE_PAUSE_PENDING);
        // ToDo: Add support for pausing.
        SetServiceStatus(SERVICE_PAUSED);
    }
    catch (DWORD errorCode)
    {
        WriteErrorLogEntry(L"Service Pause", errorCode);
        SetServiceStatus(SERVICE_RUNNING);
    }
    catch (...)
    {
        WriteEventLogEntry(L"Service failed to pause.", EVENTLOG_ERROR_TYPE);
        SetServiceStatus(SERVICE_RUNNING);
    }
}

void CServiceBase::Continue()
{
    TRACE("CServiceBase::Continue()");

    try
    {
        SetServiceStatus(SERVICE_CONTINUE_PENDING);
        // ToDo: Add support for continuing.
        SetServiceStatus(SERVICE_RUNNING);
    }
    catch (DWORD errorCode)
    {
        WriteErrorLogEntry(L"Service Continue", errorCode);
        SetServiceStatus(SERVICE_PAUSED);
    }
    catch (...)
    {
        WriteEventLogEntry(L"Service failed to resume.", EVENTLOG_ERROR_TYPE);
        SetServiceStatus(SERVICE_PAUSED);
    }
}

void CServiceBase::Shutdown()
{
    TRACE("CServiceBase::Shutdown()");

    try
    {
        // ToDo: Add support for shutting down.
        SetServiceStatus(SERVICE_STOPPED);
    }
    catch (DWORD errorCode)
    {
        WriteErrorLogEntry(L"Service Shutdown", errorCode);
    }
    catch (...)
    {
        WriteEventLogEntry(L"Service failed to shut down.", EVENTLOG_ERROR_TYPE);
    }
}

void CServiceBase::SetServiceStatus(DWORD currentState, DWORD win32ExitCode)
{
    static DWORD checkPoint = 1;

    _status.dwCurrentState = currentState;
    _status.dwWin32ExitCode = win32ExitCode;
    _status.dwWaitHint = 0;
    _status.dwCheckPoint = ((currentState == SERVICE_RUNNING) || (currentState == SERVICE_STOPPED)) ? 0 : checkPoint++;

    ::SetServiceStatus(_statusHandle, &_status);
}

void CServiceBase::WriteEventLogEntry(const wstring& message, WORD type)
{
    HANDLE eventSource = RegisterEventSource(NULL, _name.c_str());
    if (eventSource)
    {
        LPCWSTR strings[2] = { NULL, NULL };
        strings[0] = _name.c_str();
        strings[1] = message.c_str();

        ReportEvent(eventSource,   // Event log handle
            type,                  // Event type
            0,                     // Event category
            0,                     // Event identifier
            NULL,                  // No security identifier
            2,                     // Size of strings array
            0,                     // No binary data
            strings,               // Array of strings
            NULL                   // No binary data
            );

        DeregisterEventSource(eventSource);
    }
}

void CServiceBase::WriteErrorLogEntry(const wstring& function, DWORD errorCode)
{
    basic_ostringstream<wchar_t> message;
    message << function << L" failed w/err " << errorCode;
    WriteEventLogEntry(message.str(), EVENTLOG_ERROR_TYPE);
}