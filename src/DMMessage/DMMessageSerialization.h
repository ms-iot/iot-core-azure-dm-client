#pragma once

#include "DMMessageKind.h"
#include "Models\AllModels.h"

using namespace Platform;
using namespace Platform::Metadata;
using namespace Windows::Data::Json;
using namespace Microsoft::Devices::Management::Message;

namespace Microsoft { namespace Devices { namespace Management { namespace Message
{
    private class DMMessageDeserializer
    {
    public:
        typedef IDataPayload^ (DMRequestDeserialize)(Blob^ bytes);
        typedef IDataPayload^ (DMResponseDeserialize)(Blob^ bytes);
        typedef std::pair<DMRequestDeserialize*, DMResponseDeserialize*> DMSerializationPair;
        typedef std::map<DMMessageKind, DMSerializationPair> DMDeserializer;

        DMDeserializer Deserializer = {
#define MODEL_NODEF(A, B, C, D) { DMMessageKind::##A, { (DMRequestDeserialize*)##C##::Deserialize, (DMResponseDeserialize*)##D##::Deserialize } },
#define MODEL_REQDEF(A, B, C, D) MODEL_NODEF(A, B, C, D)
#define MODEL_ALLDEF(A, B, C, D) MODEL_NODEF(A, B, C, D)
#define MODEL_TAGONLY(A, B, C, D) MODEL_NODEF(A, B, C, D)
#include "Models\ModelsInfo.dat"
#undef MODEL_NODEF
#undef MODEL_REQDEF
#undef MODEL_ALLDEF
#undef MODEL_TAGONLY
        };
    };
}}}}