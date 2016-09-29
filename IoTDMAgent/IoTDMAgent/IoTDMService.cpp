#include "stdafx.h"
#include "IoTDMService.h"
#include "ThreadPool.h"

// Device twin update interval in seconds
#define DEVICE_TWIN_UPDATE_INTERVAL 5
#define AZURE_TEST_CONNECTION_STRING "HostName=bb3-thub-4.private.azure-devices-int.net;DeviceId=gmileka00;SharedAccessKey=4UURxwh3sty+0lFKVRLAVquB/gL5EncMLPMCo6YgjG8="

IoTDMService::IoTDMService(PWSTR serviceName, BOOL canStop, BOOL canShutdown, BOOL canPauseContinue)
    : CServiceBase(serviceName, canStop, canShutdown, canPauseContinue)
{
    TRACE("IoTDMService.ctor()");

    _stopping = FALSE;

    // Create a manual-reset event that is not signaled at first to indicate 
    // the stopped signal of the service.
    _stoppedEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (_stoppedEvent == NULL)
    {
        throw GetLastError();
    }
}

IoTDMService::~IoTDMService(void)
{
    if (_stoppedEvent)
    {
        CloseHandle(_stoppedEvent);
        _stoppedEvent = NULL;
    }
}

//
//   FUNCTION: IoTDMService::OnStart(DWORD, LPWSTR *)
//
//   PURPOSE: The function is executed when a Start command is sent to the 
//   service by the SCM or when the operating system starts (for a service 
//   that starts automatically). It specifies actions to take when the 
//   service starts. In this code sample, OnStart logs a service-start 
//   message to the Application log, and queues the main service function for 
//   execution in a thread pool worker thread.
//
//   PARAMETERS:
//   * argc   - number of command line arguments
//   * lpszArgv - array of command line arguments
//
//   NOTE: A service application is designed to be long running. Therefore, 
//   it usually polls or monitors something in the system. The monitoring is 
//   set up in the OnStart method. However, OnStart does not actually do the 
//   monitoring. The OnStart method must return to the operating system after 
//   the service's operation has begun. It must not loop forever or block. To 
//   set up a simple monitoring mechanism, one general solution is to create 
//   a timer in OnStart. The timer would then raise events in your code 
//   periodically, at which time your service could do its monitoring. The 
//   other solution is to spawn a new thread to perform the main service 
//   functions, which is demonstrated in this code sample.
//
void IoTDMService::OnStart(DWORD argc, LPWSTR *lpszArgv)
{
    TRACE("IoTDMService.OnStart()");

    // Log a service start message to the Application log.
    WriteEventLogEntry(L"IoTDMAgent in OnStart", EVENTLOG_INFORMATION_TYPE);

    // Queue the main service function for execution in a worker thread.
    CThreadPool::QueueUserWorkItem(&IoTDMService::ServiceWorkerThread, this);
}

//
//   FUNCTION: IoTDMService::ServiceWorkerThread(void)
//
//   PURPOSE: The method performs the main function of the service. It runs 
//   on a thread pool worker thread.
//
void IoTDMService::ServiceWorkerThread(void)
{
    TRACE("IoTDMService.ServiceWorkerThread()");

    bool initialized = false;
    while (!_stopping)
    {
        TRACE("IoTDMService.ServiceWorkerThread()->Loop");

        // Perform main service function here...
        if (!initialized)
        {
            _cloudAgent.Setup(AZURE_TEST_CONNECTION_STRING);
            initialized = true;
        }
        
        // Sync the device twin...
        _cloudAgent.SetTotalMemoryMB(_localAgent.GetTotalMemoryMB());
        _cloudAgent.SetAvailableMemoryMB(_localAgent.GetAvailableMemoryMB());
        _cloudAgent.SetBatteryLevel(_localAgent.GetBatteryLevel());
        _cloudAgent.SetBatteryStatus(_localAgent.GetBatteryStatus());
        _cloudAgent.ReportProperties();

        ::Sleep(DEVICE_TWIN_UPDATE_INTERVAL * 1000);
    }

    TRACE("IoTDMService.ServiceWorkerThread()->Done.");
    _cloudAgent.Shutdown();

    // Signal the stopped event.
    SetEvent(_stoppedEvent);
}

//
//   FUNCTION: IoTDMService::OnStop(void)
//
//   PURPOSE: The function is executed when a Stop command is sent to the 
//   service by SCM. It specifies actions to take when a service stops 
//   running. In this code sample, OnStop logs a service-stop message to the 
//   Application log, and waits for the finish of the main service function.
//
//   COMMENTS:
//   Be sure to periodically call ReportServiceStatus() with 
//   SERVICE_STOP_PENDING if the procedure is going to take long time. 
//
void IoTDMService::OnStop()
{
    TRACE("IoTDMService.OnStop()");

    // Log a service stop message to the Application log.
    WriteEventLogEntry(L"IoTDMAgent in OnStop", EVENTLOG_INFORMATION_TYPE);

    // Indicate that the service is stopping and wait for the finish of the 
    // main service function (ServiceWorkerThread).
    _stopping = TRUE;
    if (WaitForSingleObject(_stoppedEvent, INFINITE) != WAIT_OBJECT_0)
    {
        throw GetLastError();
    }
}