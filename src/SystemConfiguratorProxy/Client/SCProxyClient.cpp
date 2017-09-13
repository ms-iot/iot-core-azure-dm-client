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

IResponse^ SCProxyClient::SendCommand(IRequest^ command)
{
    auto blob = command->Serialize();
    auto json = blob->PayloadAsString;
    auto blobTag = blob->Tag;
    

    auto requestType = (UINT32)command->Tag;
    CComBSTR request = (wchar_t*)json->Data();

    try
    {
        UINT responseType = (UINT32)command->Tag;
        CComBSTR response = NULL;

        ::SendRequest(
            /* [in] */ this->hRpcBinding,
            /* [in] */ requestType,
            /* [in] */ request,
            /* [out] */ &responseType,
            /* [out] */ &response
        );

        auto responseString = ref new Platform::String(response);
        auto ret = Blob::CreateFromJson(responseType, responseString)->MakeIResponse();
        return ret;
    }
    catch (Platform::Exception^ e)
    {
        return ref new ErrorResponse(ErrorSubSystem::DeviceManagement, e->HResult, e->Message);
    }
    catch (...)
    {
        return ref new ErrorResponse(ErrorSubSystem::DeviceManagement, E_FAIL, L"Failure in SystemConfigurator SendRequest RPC");
    }
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


SCProxyClient::~SCProxyClient()
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