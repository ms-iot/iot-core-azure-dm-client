//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

// There is an error in the system header files that incorrectly
// places RpcStringBindingCompose in the app partition.
// Work around it by changing the WINAPI_FAMILY to desktop temporarily.
#pragma push_macro("WINAPI_FAMILY")
#undef WINAPI_FAMILY
#define WINAPI_FAMILY WINAPI_FAMILY_DESKTOP_APP
#include "SystemConfiguratorProxyClient.h"
#pragma pop_macro("WINAPI_FAMILY")

#include <ppltasks.h>

using namespace concurrency;
using namespace SystemConfiguratorProxyClientLib;

Windows::Foundation::IAsyncOperation<IResponse^>^ SystemConfiguratorProxyClient::SendCommandAsync(IRequest^ command)
{
    return create_async([this, command]() -> IResponse^ {

        return SendCommand(command);

    });
}

IResponse^ SystemConfiguratorProxyClient::SendCommand(IRequest^ command)
{
    auto blob = command->Serialize();
    auto bytes = blob->GetByteArrayForSerialization();

    //
    // TODO: this code is lifted (not shared) from SerializationHelper.cpp ... it should be shared somehow
    //
        int PrefixSize = 2 * sizeof(uint32_t);
        auto blobString = ref new Platform::String(reinterpret_cast<wchar_t*>(bytes->Data + PrefixSize), (bytes->Length - PrefixSize) / sizeof(wchar_t));
    //
    //
    //

    UINT32 requestType = (UINT32)command->Tag;
    BSTR request = (wchar_t*)blobString->Data();

    UINT responseType = (UINT32)command->Tag;
    UINT status = (UINT32)0;
    BSTR response = L"";

    ::SendRequest(
        /* [in] */ this->hRpcBinding,
        /* [in] */ requestType,
        /* [in] */ request,
        /* [out] */ &responseType,
        /* [out] */ &response);
    
    auto responseString = ref new Platform::String(response);

    //
    // TODO: this code is lifted (not shared) from SerializationHelper.cpp ... it should be shared somehow
    //
        auto byteptr = (const byte*)responseString->Data();
        size_t size = responseString->Length() * sizeof(wchar_t);
        size_t byteCount = PrefixSize + size;
        auto byteArray = ref new Platform::Array<byte>(static_cast<unsigned int>(byteCount));
        // First, put out the version (32 bits)
        uint32_t version = 1; // TODO: CurrentVersion;
        memcpy_s(byteArray->Data, byteCount, &version, sizeof(version));
        // Second, put out the 32-bit tag
        memcpy_s(byteArray->Data + sizeof(version), byteCount, &responseType, sizeof(responseType));
        // Followed by the serialized object:
        memcpy_s(byteArray->Data + PrefixSize, byteCount, byteptr, size);
        auto ret = Blob::CreateFromByteArray(byteArray)->MakeIResponse();
    //
    //
    //

    return ret;
}


__int64 SystemConfiguratorProxyClient::Initialize()
{
    RPC_STATUS status;
    RPC_WSTR pszStringBinding = nullptr;

    status = RpcStringBindingCompose(
        NULL,
        reinterpret_cast<RPC_WSTR>(L"ncalrpc"),
        NULL,
        reinterpret_cast<RPC_WSTR>(RPC_STATIC_ENDPOINT),
        NULL,
        &pszStringBinding);

    if (status)
    {
        goto error_status;
    }

    status = RpcBindingFromStringBinding(
        pszStringBinding, 
        &hRpcBinding);

    if (status)
    {
        goto error_status;
    }

    status = RpcStringFree(&pszStringBinding);

    if (status)
    {
        goto error_status;
    }

    RpcTryExcept
    {
        ::RemoteOpen(hRpcBinding, &phContext);
    }
    RpcExcept(1)
    {
        status = RpcExceptionCode();
    }
    RpcEndExcept

error_status:

    return status;
}


SystemConfiguratorProxyClient::~SystemConfiguratorProxyClient()
{
    RPC_STATUS status;

    if (hRpcBinding != NULL) 
    {
        RpcTryExcept
        {
            ::RemoteClose(&phContext);
        }
        RpcExcept(1)
        {
            // Ignoring the result of RemoteClose as nothing can be
            // done on the client side with this return code
            status = RpcExceptionCode();
        }
        RpcEndExcept

        status = RpcBindingFree(&hRpcBinding);
        hRpcBinding = NULL;
    }
}

///******************************************************/
///*         MIDL allocate and free                     */
///******************************************************/

void __RPC_FAR * __RPC_USER midl_user_allocate(size_t len)
{
    return(malloc(len));
}

void __RPC_USER midl_user_free(void __RPC_FAR * ptr)
{
    free(ptr);
}