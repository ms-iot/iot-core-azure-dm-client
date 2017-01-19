#pragma once

#include <stdint.h>

using namespace Platform;
using namespace Windows::Data::Json;

namespace Microsoft { namespace Devices { namespace Management { namespace Message
{
    ref class Blob;

    class SerializationHelper
    {
        // The Prefix has to 32-bit integers: the version and the tag
        static constexpr int      PrefixSize = 2 * sizeof(uint32_t);

    public:

        static Blob^ CreateEmptyBlob(uint32_t tag);
        static Blob^ CreateBlobFromPtrSize(uint32_t tag, const uint8_t* byteptr, size_t size);
        static Blob^ CreateBlobFromJson(uint32_t tag, JsonObject^ jsonObject);
        static Blob^ CreateBlobFromByteArray(uint32_t tag, const Array<uint8_t>^ bytes);

        static String^ GetStringFromBlob(const Blob^ blob);
        static void ReadDataFromBlob(const Blob^ blob, uint8_t* buffer, size_t size);
    };

}}}}