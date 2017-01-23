#pragma once

#include "DMMessageKind.h"
#include "Models\AllModels.h"

using namespace Platform;
using namespace Platform::Metadata;
using namespace Windows::Data::Json;



namespace Microsoft { namespace Devices { namespace Management { namespace Message
{
    private class DMMessageDeserializer
    {
    public:
        typedef IDataPayload^ (*DMRequestDeserialize)(Blob^ bytes);
        typedef IDataPayload^ (*DMResponseDeserialize)(Blob^ bytes);
        typedef std::pair<DMRequestDeserialize*, DMResponseDeserialize*> DMSerializationPair;
        typedef std::map<DMMessageKind, DMSerializationPair> DMDeserializer;

        DMDeserializer Deserializer = {
            //{ DMMessageKind::FactoryReset, { (DMRequestDeserialize*)&XXX::Deserialize, (DMResponseDeserialize*)&XXX::Deserialize } },
            { DMMessageKind::CheckUpdates, { (DMRequestDeserialize*)&CheckForUpdatesRequest::Deserialize, (DMResponseDeserialize*)&CheckForUpdatesResponse::Deserialize } },

            //{ DMMessageKind::ListApps, { (DMRequestDeserialize*)&XXX::Deserialize, (DMResponseDeserialize*)&XXX::Deserialize } },
            { DMMessageKind::InstallApp, { (DMRequestDeserialize*)&AppInstallRequest::Deserialize, (DMResponseDeserialize*)&AppInstallResponse::Deserialize } },
            { DMMessageKind::UninstallApp, { (DMRequestDeserialize*)&AppUninstallRequest::Deserialize, (DMResponseDeserialize*)&AppUninstallResponse::Deserialize } },
            //{ DMMessageKind::GetStartupForegroundApp, { (DMRequestDeserialize*)&XXX::Deserialize, (DMResponseDeserialize*)&XXX::Deserialize } },
            //{ DMMessageKind::ListStartupBackgroundApps, { (DMRequestDeserialize*)&XXX::Deserialize, (DMResponseDeserialize*)&XXX::Deserialize } },
            { DMMessageKind::AddStartupApp, { (DMRequestDeserialize*)&StartupAppRequest::Deserialize, (DMResponseDeserialize*)&StartupAppResponse::Deserialize } },
            { DMMessageKind::RemoveStartupApp, { (DMRequestDeserialize*)&StartupAppRequest::Deserialize, (DMResponseDeserialize*)&StartupAppResponse::Deserialize } },
            { DMMessageKind::AppLifcycle, { (DMRequestDeserialize*)&AppLifecycleRequest::Deserialize, (DMResponseDeserialize*)&AppLifecycleResponse::Deserialize } },

            { DMMessageKind::RebootSystem, { (DMRequestDeserialize*)&RebootRequest::Deserialize, (DMResponseDeserialize*)&StatusCodeResponse::Deserialize } },
            //{ DMMessageKind::SetRebootInfo, { (DMRequestDeserialize*)&XXX::Deserialize, (DMResponseDeserialize*)&XXX::Deserialize } },
            //{ DMMessageKind::GetRebootInfo, { (DMRequestDeserialize*)&XXX::Deserialize, (DMResponseDeserialize*)&XXX::Deserialize } },

            { DMMessageKind::TransferFile, { (DMRequestDeserialize*)&AzureFileTransferRequest::Deserialize, (DMResponseDeserialize*)&AzureFileTransferResponse::Deserialize } },

            { DMMessageKind::GetTimeInfo, { (DMRequestDeserialize*)&TimeInfoRequest::Deserialize, (DMResponseDeserialize*)&TimeInfoResponse::Deserialize } },
            //{ DMMessageKind::SetTimeInfo, { (DMRequestDeserialize*)&XXX::Deserialize, (DMResponseDeserialize*)&XXX::Deserialize } },

            //{ DMMessageKind::GetDeviceStatus, { (DMRequestDeserialize*)&XXX::Deserialize, (DMResponseDeserialize*)&XXX::Deserialize } },
        };
    };
}}}}