#pragma once

#include "DMMessageKind.h"
#include "ResponseStatus.h"

using namespace Platform;
using namespace Windows::Data::Json;

namespace Microsoft { namespace Devices { namespace Management { namespace Message
{
    ref class Blob;

    public interface class IDataPayload
    {
        Blob^ Serialize();
        property DMMessageKind Tag { DMMessageKind get(); }
    };

    public interface class IRequest : public IDataPayload
    {
    };

    public interface class IResponse : public IDataPayload
    {
        property ResponseStatus Status { ResponseStatus get(); }
    };

}}}}