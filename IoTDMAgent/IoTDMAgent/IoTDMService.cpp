#include "stdafx.h"
#include <thread>
#include "IoTDMService.h"
#include "LocalAgent\LocalAgent.h"

// Device twin update interval in seconds
#define DEVICE_TWIN_UPDATE_INTERVAL 5
#define AZURE_TEST_CONNECTION_STRING "<Device Connection String>"

using namespace std;

IoTDMService* IoTDMService::_this;

IoTDMService::IoTDMService(PWSTR serviceName, BOOL canStop, BOOL canShutdown, BOOL canPauseContinue)
    : CServiceBase(serviceName, canStop, canShutdown, canPauseContinue)
{
    TRACE("IoTDMService.ctor()");

    _this = this;
    _stopping = false;
}

void IoTDMService::OnStart(DWORD argc, LPWSTR *lpszArgv)
{
    TRACE("IoTDMService.OnStart()");

    thread dmThread(ServiceWorkerThread);
    dmThread.detach();
}

void IoTDMService::ServiceWorkerThread(void)
{
    _this->ServiceWorkerThreadHelper();
}

void IoTDMService::ServiceWorkerThreadHelper(void)
{
    TRACE("IoTDMService.ServiceWorkerThread()");

    if (_cloudAgent.Setup(AZURE_TEST_CONNECTION_STRING))
    {
        while (!_stopping)
        {
            TRACE("IoTDMService.ServiceWorkerThread()->Loop");

            // Sync the device twin...
            _cloudAgent.SetTotalMemoryMB(LocalAgent::GetTotalMemoryMB());
            _cloudAgent.SetAvailableMemoryMB(LocalAgent::GetAvailableMemoryMB());
            _cloudAgent.SetBatteryLevel(LocalAgent::GetBatteryLevel());
            _cloudAgent.SetBatteryStatus(LocalAgent::GetBatteryStatus());
            _cloudAgent.ReportProperties();

            ::Sleep(DEVICE_TWIN_UPDATE_INTERVAL * 1000);
        }

        TRACE("IoTDMService.ServiceWorkerThread()->Done.");
        _cloudAgent.Shutdown();
    }
    else
    {
        // If the connection to the cloud fails, there is not much the DM can do.
        TRACE("Error: Failed to setup the azure cloud agent!");
    }

    _workDone.notify_one();
}

void IoTDMService::OnStop()
{
    TRACE("IoTDMService.OnStop()");

    _stopping = true;
    unique_lock<mutex> lk(_mutex);
    _workDone.wait(lk);
}