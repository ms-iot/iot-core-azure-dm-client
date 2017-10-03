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
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include "SystemConfiguratorProxy_h.h"
#include <windows.h>

#include <sddl.h>
#include <securitybaseapi.h>
#include <AclAPI.h>
#include "SystemConfiguratorProxy.h"

#include "Models\ErrorResponse.h"
#include "DMException.h"
#include "Logger.h"
#include "Utils.h"
#include "Blob.h"

using namespace Microsoft::Devices::Management::Message;
using namespace std;

IResponse^ ProcessCommand(IRequest^ request);

#define RPC_STATIC_ENDPOINT L"IotDmRpcEndpoint"
const WCHAR* CustomCapabilityName = L"systemManagement";

static RPC_BINDING_VECTOR* BindingVector = nullptr;

void FreeSidArray(__inout_ecount(cSIDs) PSID* pSIDs, ULONG cSIDs)
{
    if (pSIDs != nullptr)
    {
        for (ULONG i = 0; i < cSIDs; i++)
        {
            LocalFree(pSIDs[i]);

            pSIDs[i] = nullptr;
        }

        LocalFree(pSIDs);

        pSIDs = nullptr;
        cSIDs = 0;
    }
}

//
// Routine to create RPC server and listen to incoming RPC calls
//
DWORD SystemConfiguratorProxyStart()
{
    TRACE("SystemConfiguratorProxyStart started...");

    DWORD hResult = S_OK;
    WCHAR* protocolSequence = L"ncalrpc";
    unsigned int minCalls = 1;
    unsigned int dontWait = false;

    SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;
    PSID everyoneSid = nullptr;
    PSID* capabilitySids = nullptr;
    DWORD capabilitySidCount = 0;
    PSID* capabilityGroupSids = nullptr;
    DWORD capabilityGroupSidCount = 0;
    EXPLICIT_ACCESS ea[2] = {};
    PACL acl = nullptr;
    SECURITY_DESCRIPTOR rpcSecurityDescriptor = {};

    TRACEP(L"DeriveCapabilitySidsFromName for: ", CustomCapabilityName);

    // Get the SID form of the custom capability.  In this case we only expect one SID and
    // we don't care about the capability group. 
    if (!DeriveCapabilitySidsFromName(
        CustomCapabilityName,
        &capabilityGroupSids,
        &capabilityGroupSidCount,
        &capabilitySids,
        &capabilitySidCount))
    {
        hResult = GetLastError();
        goto end;
    }

    if (capabilitySidCount != 1)
    {
        TRACEP(L"Failed to get SID for capability: ", CustomCapabilityName);
        // Unexpected sid count
        hResult = ERROR_INVALID_PARAMETER;
        goto end;
    }

    TRACEP(L"AllocateAndInitializeSid: ", "Everyone");
    if (!AllocateAndInitializeSid(
        &SIDAuthWorld, 1,
        SECURITY_WORLD_RID,
        0, 0, 0, 0, 0, 0, 0,
        &everyoneSid))
    {
        TRACEP(L"Failed to get SID for user: ", "Everyone");
        hResult = GetLastError();
        goto end;
    }

    // Everyone GENERIC_ALL access
    ea[0].grfAccessMode = SET_ACCESS;
    ea[0].grfAccessPermissions = GENERIC_ALL;
    ea[0].grfInheritance = NO_INHERITANCE;
    ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea[0].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
    ea[0].Trustee.ptstrName = static_cast<LPWSTR>(everyoneSid);
    // Custom capability GENERIC_ALL access
    ea[1].grfAccessMode = SET_ACCESS;
    ea[1].grfAccessPermissions = GENERIC_ALL;
    ea[1].grfInheritance = NO_INHERITANCE;
    ea[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea[1].Trustee.TrusteeType = TRUSTEE_IS_UNKNOWN;
    ea[1].Trustee.ptstrName = static_cast<LPWSTR>(capabilitySids[0]); // Earlier we ensured there was exactly 1

    TRACE("Calling SetEntriesInAcl");
    hResult = SetEntriesInAcl(ARRAYSIZE(ea), ea, nullptr, &acl);
    if (hResult != ERROR_SUCCESS)
    {
        goto end;
    }

    TRACE("Calling InitializeSecurityDescriptor");
    if (!InitializeSecurityDescriptor(&rpcSecurityDescriptor, SECURITY_DESCRIPTOR_REVISION))
    {
        hResult = GetLastError();
        goto end;
    }

    TRACE("Calling SetSecurityDescriptorDacl");
    if (!SetSecurityDescriptorDacl(&rpcSecurityDescriptor, TRUE, acl, FALSE))
    {
        hResult = GetLastError();
        goto end;
    }

    TRACE("Calling RpcServerUseProtseqEp");
    hResult = RpcServerUseProtseqEp(
        reinterpret_cast<RPC_WSTR>(protocolSequence),
        RPC_C_PROTSEQ_MAX_REQS_DEFAULT,
        reinterpret_cast<RPC_WSTR>(RPC_STATIC_ENDPOINT),
        &rpcSecurityDescriptor);

    if (hResult != S_OK)
    {
        goto end;
    }

    TRACE("Calling RpcServerRegisterIf3");
    hResult = RpcServerRegisterIf3(
        SystemConfiguratorProxyInterface_v1_0_s_ifspec,
        nullptr,
        nullptr,
        RPC_IF_AUTOLISTEN | RPC_IF_ALLOW_LOCAL_ONLY,
        RPC_C_LISTEN_MAX_CALLS_DEFAULT,
        0,
        nullptr,
        &rpcSecurityDescriptor);
    if (hResult != S_OK)
    {
        goto end;
    }

    TRACE("Calling RpcServerInqBindings");
    hResult = RpcServerInqBindings(&BindingVector);
    if (hResult != S_OK)
    {
        goto end;
    }

    TRACE("Calling RpcEpRegister");
    hResult = RpcEpRegister(
        SystemConfiguratorProxyInterface_v1_0_s_ifspec,
        BindingVector,
        nullptr,
        nullptr);
    if (hResult != S_OK)
    {
        goto end;
    }

    TRACE("Calling RpcServerListen");
    hResult = RpcServerListen(
        minCalls,
        RPC_C_LISTEN_MAX_CALLS_DEFAULT,
        dontWait);
    if (hResult == RPC_S_ALREADY_LISTENING)
    {
        hResult = RPC_S_OK;
    }

end:

    // Cleanup sids
    FreeSidArray(capabilityGroupSids, capabilityGroupSidCount);
    FreeSidArray(capabilitySids, capabilitySidCount);

    if (everyoneSid != nullptr)
    {
        FreeSid(everyoneSid);
    }

    // cleanup acl
    if (acl != nullptr)
    {
        LocalFree(acl);
    }

    TRACEP("SystemConfiguratorProxyStart ended: ", hResult);
    return hResult;
}

//
// Notify rpc server to stop listening to incoming rpc calls
//
void SystemConfiguratorProxyDisconnect()
{
    TRACE("SystemConfiguratorProxyDisconnect");

    RpcServerUnregisterIf(SystemConfiguratorProxyInterface_v1_0_s_ifspec, nullptr, 0);
    RpcEpUnregister(SystemConfiguratorProxyInterface_v1_0_s_ifspec, BindingVector, nullptr);
    RpcMgmtStopServerListening(nullptr);

    if (BindingVector != nullptr)
    {
        RpcBindingVectorFree(&BindingVector);
        BindingVector = nullptr;
    }
}

//
// Rpc method to send request to DM service
//
HRESULT SendRequest(
    _In_ handle_t /*phContext*/,
    _In_ UINT32 requestType,
    _In_ BSTR requestJson,
    __RPC__deref_out_opt UINT32* responseType,
    __RPC__deref_out_opt BSTR* responseJson
    )
{
    IResponse^ response = nullptr;
    try
    {
        TRACE("Request received...");
        TRACEP(L"    ", Utils::ConcatString(L"request tag:", (uint32_t)requestType));
        TRACEP(L"    ", Utils::ConcatString(L"request json:", requestJson));
        auto requestJsonString = ref new String(requestJson);
        auto requestBlob = Blob::CreateFromJson(requestType, requestJsonString);

        IRequest^ request = requestBlob->MakeIRequest();
        response = ProcessCommand(request);
    }
    catch (const DMExceptionWithErrorCode& e)
    {
        response = CreateErrorResponse(ErrorSubSystem::DeviceManagement, e.ErrorCode(), e.what());
    }
    catch (const exception& e)  // Note that DMException is just 'exception' with some trace statements.
    {
        response = CreateErrorResponse(ErrorSubSystem::DeviceManagement, static_cast<int>(DeviceManagementErrors::GenericError), e.what());
    }
    catch (Platform::Exception^ e)
    {
        response = ref new ErrorResponse(ErrorSubSystem::DeviceManagement, e->HResult, e->Message);
    }
    catch (...)
    {
        response = ref new ErrorResponse(ErrorSubSystem::DeviceManagement, static_cast<int>(DeviceManagementErrors::GenericError), L"Unknown exception!");
    }

    *responseType = (UINT32)response->Tag;
    auto responseJsonString = response->Serialize()->PayloadAsString;
    *responseJson = SysAllocString(responseJsonString->Data());
    TRACE("Response generated...");
    TRACEP(L"response tag :", (uint32_t)responseType);
    TRACEP(L"response json: ", responseJsonString->Data());
    return S_OK;
}

/******************************************************/
/*         MIDL allocate and free                     */
/******************************************************/

void __RPC_FAR * __RPC_USER midl_user_allocate(_In_ size_t len)
{
    return(malloc(len));
}

void __RPC_USER midl_user_free(_In_ void __RPC_FAR* ptr)
{
    free(ptr);
}
