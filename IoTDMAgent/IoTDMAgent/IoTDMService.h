#pragma once

#include "ServiceBase.h"
#include "..\LocalAgent\LocalAgent.h"
#include "AzureAgent.h"

class IoTDMService : public CServiceBase
{
public:

    IoTDMService(PWSTR serviceName,
        BOOL canStop = TRUE,
        BOOL canShutdown = TRUE,
        BOOL canPauseContinue = FALSE);
    virtual ~IoTDMService(void);

protected:

    virtual void OnStart(DWORD argc, PWSTR *argv);
    virtual void OnStop();

    void ServiceWorkerThread(void);

private:

    BOOL _stopping;
    HANDLE _stoppedEvent;

    AzureAgent _cloudAgent;
    LocalAgent _localAgent;
};