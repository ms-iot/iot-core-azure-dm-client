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

#include "stdafx.h"
#include "Models\AllModels.h"
#include "Blob.h"
#include "DMMessageSerialization.h"
#include "../SharedUtilities/Logger.h"

using namespace Platform;
using namespace concurrency;

namespace Microsoft { namespace Devices { namespace Management { namespace Message
{
    IDataPayload^ Blob::MakeMessage(MessageType messageType)
    {
        auto tag = this->Tag;

        DMMessageDeserializer helper;
        auto serialization = helper.Deserializer.find(tag);
        if (serialization == helper.Deserializer.end())
        {
            throw ref new Platform::Exception(S_OK, "Unknown type, deserialization failed");
        }

        if (messageType == MessageType::Request) {
            return (*serialization->second.first)(this);
        }
        else
        {
            return (*serialization->second.second)(this);
        }
    }

    Blob^ Blob::CreateFromByteArray(const Array<uint8_t>^ bytes)
    {
        return ref new Blob(bytes);
    }

    Blob^ Blob::ReadFromNativeHandle(uint64_t handle)
    {
        TRACE(__FUNCTION__);

        HANDLE pipeHandle = (HANDLE)handle;
        DWORD readByteCount = 0;
        uint32_t totalSizInBytes = 0;
        if (!ReadFile(pipeHandle, &totalSizInBytes, sizeof(uint32_t), &readByteCount, NULL))
        {
            throw ref new Exception(GetLastError(), "ReadFile() failed to read payload size from pipe.");
        }

        if (readByteCount != sizeof(uint32_t))
        {
            throw ref new Exception(E_FAIL, "Payload size could not be read.");
        }

        auto bytes = ref new Array<uint8_t>(totalSizInBytes);

        if (!ReadFile(pipeHandle, bytes->Data, totalSizInBytes, &readByteCount, NULL))
        {
            throw ref new Exception(GetLastError(), "ReadFile() failed to read payload from pipe");
        }

        FlushFileBuffers(pipeHandle);

        return CreateFromByteArray(bytes);
    }

    void ValidateDataSize(uint32_t minexpected, uint32_t actual)
    {
        if (minexpected < actual) {
            throw ref new Exception(E_FAIL, "Cannot read enough bytes from the stream");
        }
    }

    IAsyncOperation<Blob^>^ Blob::ReadFromIInputStreamAsync(Windows::Storage::Streams::IInputStream^ iistream)
    {
        TRACE(__FUNCTION__);

        DataReader^ reader = ref new DataReader(iistream);
        return create_async([=]() {
            return create_task(reader->LoadAsync(sizeof(uint32_t))).then([=](uint32_t bytesLoaded) {
                ValidateDataSize(sizeof(uint32_t), bytesLoaded);
                auto dataSizeArry = ref new Array<byte>(sizeof(uint32_t));
                reader->ReadBytes(dataSizeArry); // Don't try reader->ReadUInt32 here due to wrong endianness
                uint32_t dataSize;
                memcpy_s(&dataSize, sizeof(uint32_t), dataSizeArry->Data, sizeof(uint32_t));
                return create_task(reader->LoadAsync(dataSize)).then([=](uint32_t bytesLoaded) {
                    ValidateDataSize(dataSize, bytesLoaded);
                    Array<byte>^ data = ref new Array<byte>(dataSize);
                    reader->ReadBytes(data);
                    auto blob = Blob::CreateFromByteArray(data);
                    blob->ValidateVersion();
                    return blob;
                });
            });
        });
    }

    void Blob::WriteToNativeHandle(uint64_t handle)
    {
        TRACE(__FUNCTION__);

        HANDLE pipeHandle = (HANDLE)handle;

        DWORD byteWrittenCount = 0;
        uint32_t totalSizInBytes = this->bytes->Length;

        if (!WriteFile(pipeHandle, &totalSizInBytes, sizeof(uint32_t), &byteWrittenCount, NULL) || byteWrittenCount != sizeof(uint32_t))
        {
            throw ref new Exception(GetLastError(), "WriteFile() failed to write payload size to pipe.");
        }

        byteWrittenCount = 0;
        if (!WriteFile(pipeHandle, this->bytes->Data, totalSizInBytes, &byteWrittenCount, NULL))
        {
            throw ref new Exception(GetLastError(), "WriteFile() failed to write payload to pipe.");
        }

        FlushFileBuffers(pipeHandle);
    }

    IAsyncAction^ Blob::WriteToIOutputStreamAsync(IOutputStream^ iostream)
    {
        TRACE(__FUNCTION__);

        DataWriter^ writer = ref new DataWriter(iostream);
        return create_async([=]() {
            auto dataSizeArry = ref new Array<byte>(sizeof(uint32_t));
            uint32 size = this->bytes->Length;
            memcpy_s(dataSizeArry->Data, sizeof(uint32_t), &size, sizeof(uint32_t));
            writer->WriteBytes(dataSizeArry);
            return create_task(writer->StoreAsync()).then([=](auto) {
                writer->WriteBytes(this->bytes);
                return create_task(writer->StoreAsync()).then([=](auto) {
                    writer->FlushAsync();
                });
            });
        });
    }

}}}}
