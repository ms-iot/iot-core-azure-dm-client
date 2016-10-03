#pragma once
#include <thread>
#include <atomic>
#include <future>

#include "ServiceBase.h"
#include "AzureProxy.h"

class IoTDMService : public CServiceBase
{
public:

    IoTDMService(PWSTR serviceName,
        BOOL canStop = TRUE,
        BOOL canShutdown = TRUE,
        BOOL canPauseContinue = FALSE);

protected:

    virtual void OnStart(DWORD argc, PWSTR *argv);
    virtual void OnStop();

     static void ServiceWorkerThread(void* context);
     void ServiceWorkerThreadHelper();

private:
    std::atomic<bool> _stopSignaled;
    std::promise<void> _stoppedPromised;
  
    AzureAgent _cloudAgent;
};