#include "stdafx.h"
#include "Models\AllModels.h"
#include "Blob.h"

#include "..\DMMessage\GarbageCollectedTempFolder.h"
#include "DMMessageSerialization.h"

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
        else {
            return (*serialization->second.second)(this);
        }
    }

    Blob^ Blob::CreateFromByteArray(const Array<uint8_t>^ bytes)
    {
        return ref new Blob(bytes);
    }

    Blob^ Blob::ReadFromNativeHandle(IntPtr handle)
    {
        HANDLE pipeHandle = (HANDLE)handle;

        DWORD readByteCount = 0;
        uint32_t totalSizInBytes = 0;
        if (!ReadFile(pipeHandle, &totalSizInBytes, sizeof(uint32_t), &readByteCount, NULL) || readByteCount != sizeof(uint32_t))
        {
            throw ref new Exception(E_FAIL, "Cannot read buffer size from pipe");
        }

        auto bytes = ref new Array<uint8_t>(totalSizInBytes);

        if (!ReadFile(pipeHandle, bytes->Data, totalSizInBytes, &readByteCount, NULL))
        {
            throw ref new Exception(E_FAIL, "Cannot read data from pipe");
        }

        return CreateFromByteArray(bytes);

    }

    IAsyncOperation<Blob^>^ Blob::ReadFromIInputStreamAsync(Windows::Storage::Streams::IInputStream^ iistream)
    {
        DataReader^ reader = ref new DataReader(iistream);
        return create_async([=]() {
            return create_task(reader->LoadAsync(sizeof(uint32_t))).then([=](auto) {
                auto dataSizeArry = ref new Array<byte>(sizeof(uint32_t));
                reader->ReadBytes(dataSizeArry); // Don't try reader->ReadUInt32 here due to wrong endianness
                uint32_t dataSize;
                memcpy_s(&dataSize, sizeof(uint32_t), dataSizeArry->Data, sizeof(uint32_t));
                return create_task(reader->LoadAsync(dataSize)).then([=](auto) {
                    Array<byte>^ data = ref new Array<byte>(dataSize);
                    reader->ReadBytes(data);
                    auto blob = Blob::CreateFromByteArray(data);
                    blob->ValidateVersion();
                    return blob;
                });
            });
        });
    }

    void Blob::WriteToNativeHandle(IntPtr handle)
    {
        HANDLE pipeHandle = (HANDLE)handle;

        DWORD byteWrittenCount = 0;
        uint32_t totalSizInBytes = this->bytes->Length;
        if (!WriteFile(pipeHandle, &totalSizInBytes, sizeof(uint32_t), &byteWrittenCount, NULL) || byteWrittenCount != sizeof(uint32_t))
        {
            throw ref new Exception(E_FAIL, "Cannot write buffer size to pipe");
        }

        byteWrittenCount = 0;
        if (!WriteFile(pipeHandle, this->bytes->Data, totalSizInBytes, &byteWrittenCount, NULL))
        {
            throw ref new Exception(E_FAIL, "Cannot write blob to pipe");
        }
    }

    IAsyncAction^ Blob::WriteToIOutputStreamAsync(IOutputStream^ iostream)
    {
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

    String^ Blob::DMGarbageCollectedFolder::get()
    {
        return ref new String(SC_CLEANUP_FOLDER); 
    }
}}}}