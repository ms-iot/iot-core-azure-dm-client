#pragma once
#include "AppInstall.h"
#include "AppLifecycle.h"
#include "AppUninstall.h"
#include "CheckForUpdates.h"
#include "CertificateConfiguration.h"
#include "CertificateDetails.h"
#include "DeviceInfo.h"
#include "FactoryReset.h"
#include "FileTransfer.h"
#include "ListApps.h"
#include "Reboot.h"
#include "StartupApp.h"
#include "StatusCodeResponse.h"
#include "StringResponse.h"
#include "TimeInfo.h"
#include "TpmRequests.h"
#include "WindowsUpdatePolicy.h"
#include "WindowsUpdateRebootPolicy.h"
#include "WindowsUpdates.h"

#define EMPTY_REQUEST(A) \
public ref class A##Request sealed : public IRequest \
{ \
public: \
    virtual Blob^ Serialize() { \
        return SerializationHelper::CreateEmptyBlob((uint32_t)Tag); \
    } \
 \
    static IDataPayload^ Deserialize(Blob^ bytes) { \
        return ref new A##Request(); \
    } \
 \
    virtual property DMMessageKind Tag { \
        DMMessageKind get(); \
    } \
} \


namespace Microsoft { namespace Devices { namespace Management { namespace Message
{
    EMPTY_REQUEST(ListApps);
    EMPTY_REQUEST(GetStartupForegroundApp);
    EMPTY_REQUEST(ListStartupBackgroundApps);
}}}};
