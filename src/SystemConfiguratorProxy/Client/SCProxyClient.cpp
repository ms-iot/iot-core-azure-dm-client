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

// There is an error in the system header files that incorrectly
// places RpcStringBindingCompose in the app partition.
// Work around it by changing the WINAPI_FAMILY to desktop temporarily.
#pragma push_macro("WINAPI_FAMILY")
#undef WINAPI_FAMILY
#define WINAPI_FAMILY WINAPI_FAMILY_DESKTOP_APP
#include "SCProxyClient.h"
#pragma pop_macro("WINAPI_FAMILY")

#include <ppltasks.h>
#include <atlbase.h>

using namespace concurrency;
using namespace SystemConfiguratorProxyClient;

Windows::Foundation::IAsyncOperation<IResponse^>^ SCProxyClient::SendCommandAsync(IRequest^ command)
{
    return create_async([this, command]() -> IResponse^ {

        return SendCommand(command);

    });
}

DWORD DoSendCommand(handle_t binding, BSTR request, UINT requestType, BSTR *pResponse, UINT* pResponseType)
{
    if (binding == NULL)
    {
        return RPC_S_INVALID_BINDING;
    }

    RpcTryExcept
    {
        return ::SendRequest(
                /* [in] */ binding,
                /* [in] */ requestType,
                /* [in] */ request,
                /* [out] */ pResponseType,
                /* [out] */ pResponse);
    }
    RpcExcept(1)
    {
        // Ignoring the result of RemoteClose as nothing can be
        // done on the client side with this return code
        return RpcExceptionCode();
    }
    RpcEndExcept
}

IResponse^ SCProxyClient::SendCommand(IRequest^ command)
{
    auto blob = command->Serialize();
    auto json = blob->PayloadAsString;
    auto blobTag = blob->Tag;

    auto requestType = (UINT32)command->Tag;
    CComBSTR requestJson = (wchar_t*)json->Data();
    UINT responseType = (UINT32)command->Tag;
    CComBSTR responseJson = NULL;

    auto status = DoSendCommand(this->hRpcBinding, requestJson, requestType, &responseJson, &responseType);
    IResponse^ response = nullptr;
    if (RPC_S_OK != status /*implied: || S_OK != status || ERROR_SUCCESS != status*/)
    {
        // Ignoring the result of RemoteClose as nothing can be
        // done on the client side with this return code
        response = ref new ErrorResponse(ErrorSubSystem::DeviceManagement, status, L"Failure in SystemConfigurator SendRequest RPC");
    }
    else
    {
        auto responseJsonString = ref new Platform::String(responseJson);
        response = Blob::CreateFromJson(responseType, responseJsonString)->MakeIResponse();
    }

    return response;

}


__int64 SCProxyClient::Initialize()
{
    RPC_STATUS status;
    RPC_WSTR pszStringBinding = nullptr;

    status = RpcStringBindingCompose(
        NULL,
        reinterpret_cast<RPC_WSTR>(RPC_PROTOCOL),
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

error_status:

    if (pszStringBinding != nullptr)
    {
        RpcStringFree(&pszStringBinding);
    }


    return status;
}


SCProxyClient::~SCProxyClient()
{
    RPC_STATUS status;

    if (hRpcBinding != NULL) 
    {
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