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
#include "SerializationHelper.h"
#include "Blob.h"
#include "CurrentVersion.h"

using namespace Microsoft::Devices::Management::Message;
using namespace Platform;

Blob^ SerializationHelper::CreateEmptyBlob(uint32_t tag)
{
    return CreateBlobFromPtrSize(tag, nullptr, 0);
}

Blob^ SerializationHelper::CreateBlobFromPtrSize(uint32_t tag, const byte* byteptr, size_t size)
{
    size_t byteCount = PrefixSize + size;

    auto byteArray = ref new Array<byte>(static_cast<unsigned int>(byteCount));

    // First, put out the version (32 bits)
    uint32_t version = CurrentVersion;
    memcpy_s(byteArray->Data, byteCount, &version, sizeof(version));

    // Second, put out the 32-bit tag
    memcpy_s(byteArray->Data + sizeof(version), byteCount, &tag, sizeof(tag));

    // Followed by the serialized object:
    memcpy_s(byteArray->Data + PrefixSize, byteCount, byteptr, size);

    return Blob::CreateFromByteArray(byteArray);
}

Blob^ SerializationHelper::CreateBlobFromJson(uint32_t tag, JsonObject^ jsonObject)
{
    String^ str = jsonObject->Stringify();
    return CreateBlobFromString(tag, str);
}

Blob^ SerializationHelper::CreateBlobFromString(uint32_t tag, String ^str)
{
    return CreateBlobFromPtrSize(tag, (const byte*)str->Data(), str->Length() * sizeof(wchar_t));
}

Blob^ SerializationHelper::CreateBlobFromByteArray(uint32_t tag, const Array<byte>^ bytes)
{
    const byte* byteptr = bytes->Data;
    return CreateBlobFromPtrSize(tag, byteptr, bytes->Length);
}

String^ SerializationHelper::GetStringFromBlob(const Blob^ blob)
{
    return ref new String(reinterpret_cast<wchar_t*>(blob->bytes->Data + PrefixSize), (blob->bytes->Length - PrefixSize) / sizeof(wchar_t));
}

void SerializationHelper::ReadDataFromBlob(const Blob^ blob, byte* buffer, size_t size)
{
    memcpy_s(buffer, size, blob->bytes->Data + PrefixSize, size);
}
