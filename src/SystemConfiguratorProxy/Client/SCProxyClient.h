//
// RpcClient.h
//

#pragma once

#define RPC_STATIC_ENDPOINT L"IotDmRpcEndpoint"

#include "SystemConfiguratorProxy_h.h"

using namespace Microsoft::Devices::Management::Message;

namespace SystemConfiguratorProxyClient
{
    /// <summary>
    /// Client side RPC implementation
    /// </summary>
    public ref class SCProxyClient sealed
    {
    public:
        virtual ~SCProxyClient();

        Windows::Foundation::IAsyncOperation<IResponse^>^ SendCommandAsync(IRequest^ command);
        IResponse^ SendCommand(IRequest^ command);

        __int64 Initialize();

    private:
        handle_t hRpcBinding;
        PCONTEXT_HANDLE_TYPE phContext;
    };
}
