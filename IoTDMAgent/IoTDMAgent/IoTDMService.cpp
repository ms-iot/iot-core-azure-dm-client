#include "stdafx.h"
#include "IoTDMService.h"
#include "AzureProxy.h"
#include "LocalMachine\LocalMachine.h"

// Thread will check whether it is time to renew the connection string 
// every CONNECTION_RENEWAL_CHECK_INTERVAL seconds.
// Making this longer affects the time the service takes to response
// to stop requests.
// ToDo: consider waiting on multiple events.
#define CONNECTION_RENEWAL_CHECK_INTERVAL 5

using namespace std;

IoTDMService *IoTDMService::s_service = NULL;

void IoTDMService::Run(IoTDMService &service)
{
    TRACE("IoTDMService::Run()");

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

void WINAPI IoTDMService::ServiceMain(DWORD argc, PWSTR *argv)
{
    TRACE("IoTDMService::ServiceMain()");
    assert(s_service != NULL);

    s_service->_statusHandle = RegisterServiceCtrlHandler(s_service->_name.c_str(), ServiceCtrlHandler);
    if (s_service->_statusHandle == NULL)
    {
        throw GetLastError();
    }

    s_service->Start(argc, argv);
}

void WINAPI IoTDMService::ServiceCtrlHandler(DWORD ctrl)
{
    TRACE("IoTDMService::ServiceCtrlHandler()");

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

IoTDMService::IoTDMService(const wstring& serviceName) :
    _stopSignaled(false)
{
    TRACE("IoTDMService.ctor()");
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

    // Create the tasks queue
    _taskQueue = make_shared<TaskQueue>();
}

void IoTDMService::Start(DWORD argc, LPWSTR *argv)
{
    TRACE("IoTDMService::Start()");
    try
    {
        SetServiceStatus(SERVICE_START_PENDING);
        OnStart(argc, argv);
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

void IoTDMService::Stop()
{
    TRACE("IoTDMService::Stop()");

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

void IoTDMService::Pause()
{
    TRACE("IoTDMService::Pause()");

    // ToDo: Add support for pausing.
    //       New code should be in a try/catch to make sure the service recovers (set Stop() or Start())
    //       or at least reports an error to the event log.

    SetServiceStatus(SERVICE_PAUSE_PENDING);
    SetServiceStatus(SERVICE_PAUSED);
}

void IoTDMService::Continue()
{
    TRACE("IoTDMService::Continue()");

    // ToDo: Add support for continuing.
    //       New code should be in a try/catch to make sure the service recovers (set Stop() or Start())
    //       or at least reports an error to the event log.

    SetServiceStatus(SERVICE_CONTINUE_PENDING);
    SetServiceStatus(SERVICE_RUNNING);
}

void IoTDMService::Shutdown()
{
    TRACE("IoTDMService::Shutdown()");

    // ToDo: Add support for shutting down.
    //       New code should be in a try/catch to make sure the service recovers (set Stop() or Start())
    //       or at least reports an error to the event log.
    SetServiceStatus(SERVICE_STOPPED);
}

void IoTDMService::SetServiceStatus(DWORD currentState, DWORD win32ExitCode)
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

void IoTDMService::WriteEventLogEntry(const wstring& message, WORD type)
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

void IoTDMService::WriteErrorLogEntry(const wstring& function, DWORD errorCode)
{
    basic_ostringstream<wchar_t> message;
    message << function << L" failed w/err " << errorCode;
    WriteEventLogEntry(message.str(), EVENTLOG_ERROR_TYPE);
}

void IoTDMService::OnStart(DWORD argc, LPWSTR *argv)
{
    TRACE("IoTDMService.OnStart()");

    thread dmThread(ServiceWorkerThread, this);
    dmThread.detach();
}

void IoTDMService::ServiceWorkerThread(void* context)
{
    IoTDMService* iotDMService = static_cast<IoTDMService*>(context);
    iotDMService->ServiceWorkerThreadHelper();
}

void IoTDMService::RenewConnectionString(IoTDMService* pThis)
{
    TRACE(L"IoTDMService::RenewConnectionString()");

    while (!pThis->_stopSignaled)
    {
        TRACE(L"Renewing the connection string...");
        string connectionString = pThis->_connectionString.Generate();

        shared_ptr<TaskItem> taskItem = make_shared<TaskItem>();
        taskItem->id = TaskQueue::GetJobId();
        taskItem->type = TaskType::RenewConnectionStringTask;
        taskItem->request = connectionString;

        pThis->_taskQueue->Enqueue(taskItem);

        TRACE(L"RenewConnectionString() - is about to expire?");
        while (!pThis->_connectionString.IsAboutToExpire() && !pThis->_stopSignaled)
        {
            ::Sleep(CONNECTION_RENEWAL_CHECK_INTERVAL * 1000);
        }
    }

    pThis->_connectionRenewerStopped.set_value();
    TRACE("Connection string renewal thread exiting.");
}

void IoTDMService::ServiceWorkerThreadHelper(void)
{
    TRACE("IoTDMService::ServiceWorkerThreadHelper()");
    try
    {
        // Start a thread to renew the connection string when it is about to expire...
        thread connectionStringRenew(RenewConnectionString, this);
        connectionStringRenew.detach();

        // Start the main processing loop...
        unique_ptr<AzureProxy> cloudProxy;
        while (_taskQueue->IsActive())
        {
            TRACE("Worker thread waiting for a task to be queued...");
            shared_ptr<TaskItem> taskItem = _taskQueue->Dequeue();

            TRACE("A task has been dequeued...");
            switch (taskItem->type)
            {
            case TaskType::RenewConnectionStringTask:
                TRACE("Processing connection string renewal...");
                cloudProxy = make_unique<AzureProxy>(_taskQueue, taskItem->request);
                break;
            case TaskType::ExitServiceTask:
                TRACE("Processing ExitService task");
                _taskQueue->DisableEnqueue();       // Don't accept any more tasks.
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

                taskItem->response.set_value("");   // Signal the completion of the above steps.
                break;
            case TaskType::RebootTask:
                cloudProxy->Reboot();
                break;
            case TaskType::RemoteWipeTask:
                cloudProxy->RemoteWipe();
                break;
            case TaskType::ReportPropertiesTask:
                cloudProxy->ReportAllProperties();
                break;
            case TaskType::ProcessDesiredPropertiesTask:
                {
                    shared_ptr<DesiredPropertiesTaskItem> desiredPropertiesItem = dynamic_pointer_cast<DesiredPropertiesTaskItem>(taskItem);
                    assert(desiredPropertiesItem);
                    cloudProxy->ProcessDesiredProperties(desiredPropertiesItem->completeSet, desiredPropertiesItem->request);
                }
                break;
            case TaskType::UnknownTask:
                TRACE(L"Error: unexpected task item dequeued.");
                break;
            }

            // TODO: Before we destroy the existing iot client, we need to get all the un-sent messages 
            // and re-queue them with the new client.
        }
    }
    catch(exception& e)
    {
        // If the connection to the cloud fails, there is not much the DM can do.
        WriteEventLogEntry(Utils::MultibyteToWide(e.what()), EVENTLOG_ERROR_TYPE);

        // ToDo: No point of continuing at this stage.
        //       - We need to notify the service so that it exists.
        //       - Also, we need a way to get errors from the Azure thread...
    }

    _workerStopped.set_value();
    TRACE("Worker thread exiting.");
}

void IoTDMService::OnStop()
{
    TRACE("IoTDMService.OnStop()");

    TRACE("IoTDMService.OnStop() - signaling worker thread to exit...");
    std::shared_ptr<TaskItem> taskItem = std::make_shared<TaskItem>();
    taskItem->id = TaskQueue::GetJobId();
    taskItem->type = TaskType::ExitServiceTask;
    _taskQueue->Enqueue(taskItem);

    TRACE("IoTDMService.OnStop() - waiting for azure proxy to shutdown...");
    taskItem->response.get_future().wait();

    TRACE("IoTDMService.OnStop() - signaling connection string renewing string...");
    _stopSignaled = true;

    TRACE("IoTDMService.OnStop() - waiting for worker thread to exit...");
    _workerStopped.get_future().wait();

    TRACE("IoTDMService.OnStop() - waiting for connection string renewing thread to exit...");
    _connectionRenewerStopped.get_future().wait();

    TRACE("IoTDMService.OnStop() - all exited.");
}

void IoTDMService::Install(
    const wstring& serviceName,
    const wstring& displayName,
    DWORD startType,
    const wstring& dependencies,
    const wstring& account,
    const wstring& password)
{
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
    }

    // Install the service into SCM by calling CreateService
    SC_HANDLE schService = CreateService(
        schSCManager,                   // SCManager database
        serviceName.c_str(),                 // Name of service
        displayName.c_str(),                 // Name to display
        SERVICE_QUERY_STATUS,           // Desired access
        SERVICE_WIN32_OWN_PROCESS,      // Service type
        startType,                    // Service start type
        SERVICE_ERROR_NORMAL,           // Error control type
        szPath,                         // Service's binary
        NULL,                           // No load ordering group
        NULL,                           // No tag identifier
        dependencies.c_str(),                // Dependencies
        account.c_str(),                     // Service running account
        password.c_str()                     // Password of the account
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

void IoTDMService::Uninstall(const wstring& serviceName)
{
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