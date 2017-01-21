#pragma once

using namespace Platform;
using namespace Platform::Metadata;
using namespace Windows::Data::Json;

namespace Microsoft { namespace Devices { namespace Management { namespace Message
{
    [Flags]
    public enum class DMMessageKind : uint32_t
    {
        Unknown = 0,
        FactoryReset = 1,
        CheckUpdates = 2,
        ListApps = 3,
        InstallApp = 4,
        UninstallApp = 5,
        GetStartupForegroundApp = 6,
        ListStartupBackgroundApps = 7,
        AddStartupApp = 8,
        RemoveStartupApp = 9,

        // Reboot
        RebootSystem = 10,
        SetRebootInfo = 11,
        GetRebootInfo = 12,

        // TimeInfo
        GetTimeInfo = 30,
        SetTimeInfo = 31,

        // Device Status
        GetDeviceStatus = 40,

        // Generic Status response code:
        StatusResponse = 100,
    };
}}}}