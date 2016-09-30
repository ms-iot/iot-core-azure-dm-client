#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>

#include "ServiceBase.h"
#include "AzureAgent.h"

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

     static void ServiceWorkerThread(void);
     void ServiceWorkerThreadHelper(void);

private:
    static IoTDMService* _this;

    bool _stopping;

    std::mutex _mutex;
    std::condition_variable _workDone;

    AzureAgent _cloudAgent;
};