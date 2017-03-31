/*
Copyright 2017 Microsoft
Permission is hereby granted, free of charge, to any person obtaining a copy of this software 
and associated documentation files (the "Software"), to deal in the Software without restriction, 
including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, 
subject to the following conditions:

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT 
LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH 
THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
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
