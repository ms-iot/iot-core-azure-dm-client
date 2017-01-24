#pragma once
#include "AppInstall.h"
#include "AppLifecycle.h"
#include "AppUninstall.h"
#include "CheckForUpdates.h"
#include "FileTransfer.h"
#include "StartupApp.h"
#include "StatusCodeResponse.h"
#include "Reboot.h"
#include "TimeInfo.h"

#define EMPTY_REQUEST(A) \
public ref class A##Request sealed : public IRequest \
{ \
public: \
    virtual Blob^ Serialize() { \
        return SerializationHelper::CreateEmptyBlob((uint32_t)Tag); \
    } \
 \
    static IDataPayload^ Deserialize(Blob^ bytes) { \
        return ref new FactoryResetRequest(); \
    } \
 \
    virtual property DMMessageKind Tag { \
        DMMessageKind get(); \
    } \
} \


namespace Microsoft { namespace Devices { namespace Management { namespace Message
{
    EMPTY_REQUEST(FactoryReset);
    EMPTY_REQUEST(ListApps);
    EMPTY_REQUEST(GetStartupForegroundApp);
    EMPTY_REQUEST(ListStartupBackgroundApps);
    EMPTY_REQUEST(SetRebootInfo);
    EMPTY_REQUEST(GetRebootInfo);
    EMPTY_REQUEST(GetDeviceStatus);
    EMPTY_REQUEST(SetTimeInfo);
}}}};
