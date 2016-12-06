#include "stdafx.h"
#include <assert.h>
#include "DMService.h"
#include "..\SharedUtilities\DMException.h"
#include "CommandProcessor.h"

// Thread will check whether it is time to renew the connection string 
// every CONNECTION_RENEWAL_CHECK_INTERVAL seconds.
// Making this longer affects the time the service takes to response
// to stop requests.
// ToDo: consider waiting on multiple events.
#define CONNECTION_RENEWAL_CHECK_INTERVAL 5

using namespace std;

DMService *DMService::s_service = NULL;

void DMService::Run(DMService &service)
{
    TRACE(__FUNCTION__);

    s_service = &service;

    SERVICE_TABLE_ENTRY serviceTable[] =
    {
        { const_cast<LPWSTR>(service._name.c_str()), ServiceMain },
        { NULL, NULL }
    };

    if (!StartServiceCtrlDispatcher(serviceTable))
    {
        throw DMExceptionWithErrorCode(GetLastError());
    }
}

void WINAPI DMService::ServiceMain(DWORD argc, PWSTR *argv)
{
    TRACE(__FUNCTION__);
    assert(s_service != NULL);

    s_service->_statusHandle = RegisterServiceCtrlHandler(s_service->_name.c_str(), ServiceCtrlHandler);
    if (s_service->_statusHandle == NULL)
    {
        throw DMExceptionWithErrorCode(GetLastError());
    }

    s_service->Start(argc, argv);
}

void WINAPI DMService::ServiceCtrlHandler(DWORD ctrl)
{
    TRACE(__FUNCTION__);

    switch (ctrl)
    {
    case SERVICE_CONTROL_STOP:
        TRACE("Service stop received...");
        s_service->Stop();
        break;
    case SERVICE_CONTROL_PAUSE:
        TRACE("Service pause received...");
        s_service->Pause();
        break;
    case SERVICE_CONTROL_CONTINUE:
        TRACE("Service continue received...");
        s_service->Continue();
        break;
    case SERVICE_CONTROL_SHUTDOWN:
        TRACE("Service shutdown received...");
        s_service->Shutdown();
        break;
    case SERVICE_CONTROL_INTERROGATE:
        TRACE("Service interrogate received...");
        break;
    default: break;
    }
}

DMService::DMService(const wstring& serviceName) :
    _stopSignaled(false),
    _renewConnectionString(false)
{
    TRACE(__FUNCTION__);
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

void DMService::Start(DWORD argc, LPWSTR *argv)
{
    TRACE(__FUNCTION__);
    try
    {
        SetServiceStatus(SERVICE_START_PENDING);
        OnStart(argc, argv);
        SetServiceStatus(SERVICE_RUNNING);
    }
    catch (const DMException&)
    {
        WriteEventLogEntry(L"Service failed to start.", EVENTLOG_ERROR_TYPE);
        SetServiceStatus(SERVICE_STOPPED);
    }
}

void DMService::Stop()
{
    TRACE(__FUNCTION__);

    DWORD originalState = _status.dwCurrentState;
    try
    {
        SetServiceStatus(SERVICE_STOP_PENDING);
        OnStop();
        SetServiceStatus(SERVICE_STOPPED);
    }
    catch (const DMException&)
    {
        WriteEventLogEntry(L"Service failed to stop.", EVENTLOG_ERROR_TYPE);
        SetServiceStatus(originalState);
    }
}

void DMService::Pause()
{
    TRACE(__FUNCTION__);

    // ToDo: Add support for pausing.
    //       New code should be in a try/catch to make sure the service recovers (set Stop() or Start())
    //       or at least reports an error to the event log.

    SetServiceStatus(SERVICE_PAUSE_PENDING);
    SetServiceStatus(SERVICE_PAUSED);
}

void DMService::Continue()
{
    TRACE(__FUNCTION__);

    // ToDo: Add support for continuing.
    //       New code should be in a try/catch to make sure the service recovers (set Stop() or Start())
    //       or at least reports an error to the event log.

    SetServiceStatus(SERVICE_CONTINUE_PENDING);
    SetServiceStatus(SERVICE_RUNNING);
}

void DMService::Shutdown()
{
    TRACE(__FUNCTION__);

    // ToDo: Add support for shutting down.
    //       New code should be in a try/catch to make sure the service recovers (set Stop() or Start())
    //       or at least reports an error to the event log.
    SetServiceStatus(SERVICE_STOPPED);
}

void DMService::SetServiceStatus(DWORD currentState, DWORD win32ExitCode)
{
    static DWORD checkPoint = 1;

    _status.dwCurrentState = currentState;
    _status.dwWin32ExitCode = win32ExitCode;
    _status.dwWaitHint = 0;
    _status.dwCheckPoint = ((currentState == SERVICE_RUNNING) || (currentState == SERVICE_STOPPED)) ? 0 : checkPoint++;

    if (!::SetServiceStatus(_statusHandle, &_status))
    {
        WriteErrorLogEntry(L"Error: Could not set service status. Error Code ", GetLastError());
    }
}

void DMService::WriteEventLogEntry(const wstring& message, WORD type)
{
    TRACE(message.c_str());

    HANDLE eventSource = RegisterEventSource(NULL, _name.c_str());
    if (!eventSource)
    {
        TRACEP(L"Error: Failed to register the event source. Error Code ", GetLastError());
        // No exception is thrown since it is not a scenario that should affect normal operations.
        return;
    }

    LPCWSTR strings[2] = {
        _name.c_str(),
        message.c_str()
    };

    if (!ReportEvent(eventSource,   // Event log handle
        type,                  // Event type
        0,                     // Event category
        0,                     // Event identifier
        NULL,                  // No security identifier
        2,                     // Size of strings array
        0,                     // No binary data
        strings,               // Array of strings
        NULL                   // No binary data
    ))
    {
        TRACEP(L"Error: Failed to write to the event log. Error Code ", GetLastError());
        // No exception is thrown since it is not a scenario that should affect normal operations.
    }

    if (!DeregisterEventSource(eventSource))
    {
        TRACEP(L"Error: Failed to unregister the event source. Error Code ", GetLastError());
        // No exception is thrown since it is not a scenario that should affect normal operations.
    }
}

void DMService::WriteErrorLogEntry(const wstring& function, DWORD errorCode)
{
    basic_ostringstream<wchar_t> message;
    message << function << L" failed w/err " << errorCode;
    WriteEventLogEntry(message.str(), EVENTLOG_ERROR_TYPE);
}

void DMService::OnStart(DWORD argc, LPWSTR *argv)
{
    TRACE(__FUNCTION__);

    _workerThread = thread(ServiceWorkerThread, this);
}

void DMService::ServiceWorkerThread(void* context)
{
    TRACE(__FUNCTION__);

    DMService* iotDMService = static_cast<DMService*>(context);
    iotDMService->ServiceWorkerThreadHelper();
}

void DMService::DisableEnqueue()
{
#if 0
    _taskQueue.DisableEnqueue();       // Don't accept any more tasks.
                                       // - This allows the queue to become 'inactive'
                                       //   once all the already-queued items are consumed.
                                       //
                                       // If communication from the device twin takes place,
                                       // calls to enqueue them will fail.
                                       // - For methods, an error return code will be returned 
                                       //   to Azure IoTHub
                                       // - For property changes, failures will be ignored.
                                       //   This is okay because the next time the service runs,
                                       //   it will get the same properties again.
                                       //
                                       // Note also that we need to leave AzureProxy connected
                                       // because other already-queued items might require
                                       // sending data to the Azure IoTHub.
                                       // Only when we exit this loop, we can destruct AzureProxy.
                                       // 
                                       // DisableEnqueue() has to be queued in a task for two reasons:
                                       // 1. If the queue is already empty and waiting, we need to feed
                                       //    it an item to get it going and re-activate the loop.
                                       // 2. If the DisableEnqueue() happens between the IsActive() and 
                                       //    the blocked Dequeue(), there will be a deadlock.
#endif
}

void DMService::ServiceWorkerThreadHelper(void)
{
    TRACE(__FUNCTION__);
    // ToDo: Need a way to unblock this call.
    //       Right now, this thread will just die when the service exits.
    Listen();
    TRACE("Worker thread exiting.");
}

void DMService::OnStop()
{
    TRACE(__FUNCTION__);

    // ToDo: Need a graceful way to signal the work thread to exit.
}

void DMService::Install(
    const wstring& serviceName,
    const wstring& displayName,
    DWORD startType,
    const wstring& dependencies,
    const wstring& account,
    const wstring& password)
{
    TRACE(__FUNCTION__);

    wchar_t szPath[MAX_PATH];
    if (GetModuleFileName(NULL, szPath, ARRAYSIZE(szPath)) == 0)
    {
        TRACEP(L"GetModuleFileName failed w/err :", GetLastError());
        return;
    }

    SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT | SC_MANAGER_CREATE_SERVICE);
    if (schSCManager == NULL)
    {
        TRACEP(L"OpenSCManager failed w/err :", GetLastError());
        return;
    }

    // Install the service into SCM by calling CreateService
    SC_HANDLE schService = CreateService(
        schSCManager,                   // SCManager database
        serviceName.c_str(),            // Name of service
        displayName.c_str(),            // Name to display
        SERVICE_QUERY_STATUS,           // Desired access
        SERVICE_WIN32_OWN_PROCESS,      // Service type
        startType,                      // Service start type
        SERVICE_ERROR_NORMAL,           // Error control type
        szPath,                         // Service's binary
        NULL,                           // No load ordering group
        NULL,                           // No tag identifier
        dependencies.c_str(),           // Dependencies
        account.c_str(),                // Service running account
        password.c_str()                // Password of the account
    );

    if (schService != NULL)
    {
        CloseServiceHandle(schService);
        schService = NULL;
        TRACE(L"Installed successfully!");
    }
    else
    {
        TRACEP(L"CreateService failed w/err :", GetLastError());
    }

    CloseServiceHandle(schSCManager);
    schSCManager = NULL;
}

void DMService::Uninstall(const wstring& serviceName)
{
    TRACE(__FUNCTION__);

    SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (schSCManager == NULL)
    {
        TRACEP(L"OpenSCManager failed w/err :", GetLastError());
        return;
    }

    // Open the service with delete, stop, and query status permissions
    SC_HANDLE schService = OpenService(schSCManager, serviceName.c_str(), SERVICE_STOP | SERVICE_QUERY_STATUS | DELETE);
    if (schService != NULL)
    {
        // Try to stop the service
        SERVICE_STATUS ssSvcStatus = {};
        if (ControlService(schService, SERVICE_CONTROL_STOP, &ssSvcStatus))
        {
            TRACEP(L"Stopping ", serviceName);
            Sleep(1000);

            while (QueryServiceStatus(schService, &ssSvcStatus))
            {
                if (ssSvcStatus.dwCurrentState == SERVICE_STOP_PENDING)
                {
                    TRACE(L".");
                    Sleep(1000);
                }
                else
                {
                    break;
                }
            }

            if (ssSvcStatus.dwCurrentState == SERVICE_STOPPED)
            {
                TRACE(L"\nStopped.");
            }
            else
            {
                TRACE(L"\nError: Failed to stop.");
            }
        }

        // Now remove the service by calling DeleteService.
        if (DeleteService(schService))
        {
            TRACE(L"Service uninstalled.");
        }
        else
        {
            TRACEP(L"DeleteService failed w/err :", GetLastError());
        }

        CloseServiceHandle(schService);
        schService = NULL;
    }
    else
    {
        TRACEP(L"OpenService failed w/err :", GetLastError());
    }

    CloseServiceHandle(schSCManager);
    schSCManager = NULL;
}