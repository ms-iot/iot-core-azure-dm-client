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

        // Serialization
        static Blob^ ReadFromNativeHandle(IntPtr handle);
        static IAsyncOperation<Blob^>^ ReadFromIInputStreamAsync(IInputStream^ iistream);

        void WriteToNativeHandle(IntPtr handle);
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