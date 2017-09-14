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
#include <assert.h>
#include "IRequestIResponse.h"
#include "CurrentVersion.h"

using namespace Platform;
using namespace Platform::Metadata;
using namespace Windows::Foundation;
using namespace Windows::Data::Json;
using namespace Windows::Storage::Streams;

namespace Microsoft { namespace Devices { namespace Management { namespace Message
{
    enum class MessageType
    {
        Request,
        Response
    };

    //
    // Blob is a thin wrapper around a byte array with an 8-byte prefix (version + kind)
    //
    public ref class Blob sealed {
        const Array<uint8_t>^ bytes;
        Blob(const Array<uint8_t>^ bytes) : bytes(bytes)
        {
            assert(bytes->Length >= 8);
        }

        friend class SerializationHelper;
        // The prefix has to 32-bit integers: the version and the command
        static constexpr int      PrefixSize = 2 * sizeof(uint32_t);

        IDataPayload^ MakeMessage(MessageType);

    public:

        // Only used for testing, clients should not use
        static Blob^ CreateFromByteArray(const Array<uint8_t>^ bytes);

        static Blob^ CreateFromJson(uint32_t tag, String^ json);

        // Serialization
        static Blob^ ReadFromNativeHandle(uint64_t handle);
        static IAsyncOperation<Blob^>^ ReadFromIInputStreamAsync(IInputStream^ iistream);

        void WriteToNativeHandle(uint64_t handle);
        IAsyncAction^ WriteToIOutputStreamAsync(IOutputStream^ iostream);

        // Parsing
        IRequest^ MakeIRequest()   { return (IRequest^)this->MakeMessage(MessageType::Request); }
        IResponse^ MakeIResponse() { return (IResponse^)this->MakeMessage(MessageType::Response);}


        void ValidateVersion()
        {
            if (this->Version != CurrentVersion)
            {
                throw ref new Exception(E_FAIL, "Version mispatch. Check your installation");
            }
        }

        property uint32_t Version
        {
            uint32_t get()
            {
                // Return the first uint32:
                return *(reinterpret_cast<uint32_t*>(bytes->Data));
            }
        }

        property String^ PayloadAsString { String^ get(); }

        property DMMessageKind Tag
        {
            DMMessageKind get()
            {
                // Return the second uint32:
                return static_cast<DMMessageKind>(*(reinterpret_cast<uint32_t*>(bytes->Data) + 1));
            }
        }

        Array<uint8_t>^ GetByteArrayForSerialization() { return (Array<uint8_t>^)this->bytes; }
    };
}}}}
