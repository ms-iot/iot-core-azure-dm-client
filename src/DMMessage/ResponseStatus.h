#pragma once

using namespace Platform;
using namespace Platform::Metadata;
using namespace Windows::Data::Json;

namespace Microsoft { namespace Devices { namespace Management { namespace Message
{
    public enum class ResponseStatus
    {
        Success,
        Failure
    };

}}}}