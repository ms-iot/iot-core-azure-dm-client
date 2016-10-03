#include "stdafx.h"
#include <thread>
#include "IoTDMService.h"
#include "LocalMachine\LocalMachine.h"

// Device twin update interval in seconds
#define DEVICE_TWIN_UPDATE_INTERVAL 5
#define AZURE_TEST_CONNECTION_STRING "HostName=rc-test-005.private.azure-devices-int.net;DeviceId=gmileka01;SharedAccessKey=McvlYZ/NmgECyMwQOOflGbmAwjYxk/u613vD9E9mwxY="

using namespace std;

IoTDMService::IoTDMService(PWSTR serviceName, BOOL canStop, BOOL canShutdown, BOOL canPauseContinue)
    : CServiceBase(serviceName, canStop, canShutdown, canPauseContinue),
    _stopSignaled(false)
{
    TRACE("IoTDMService.ctor()");
}

void IoTDMService::OnStart(DWORD argc, LPWSTR *lpszArgv)
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

void IoTDMService::ServiceWorkerThreadHelper(void)
{
    TRACE("IoTDMService.ServiceWorkerThread()");

    if (_cloudAgent.Setup(AZURE_TEST_CONNECTION_STRING))
    {
        while (!_stopSignaled)
        {
            TRACE("IoTDMService.ServiceWorkerThread()->Loop");

            // Sync the device twin...
            _cloudAgent.SetTotalMemoryMB(LocalMachine::GetTotalMemoryMB());
            _cloudAgent.SetAvailableMemoryMB(LocalMachine::GetAvailableMemoryMB());
            _cloudAgent.SetBatteryLevel(LocalMachine::GetBatteryLevel());
            _cloudAgent.SetBatteryStatus(LocalMachine::GetBatteryStatus());
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

    _stoppedPromised.set_value();
}

void IoTDMService::OnStop()
{
    TRACE("IoTDMService.OnStop()");

    _stopSignaled = true;
    _stoppedPromised.get_future().wait();
}