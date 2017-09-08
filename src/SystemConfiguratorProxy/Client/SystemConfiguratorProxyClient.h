//
// RpcClient.h
//

#pragma once

#define RPC_STATIC_ENDPOINT L"IotDmRpcEndpoint"

#include "SystemConfiguratorProxy_h.h"

using namespace Microsoft::Devices::Management::Message;

namespace SystemConfiguratorProxyClientLib
{
    /// <summary>
    /// Client side RPC implementation
    /// </summary>
    public ref class SystemConfiguratorProxyClient sealed
    {
    public:
        virtual ~SystemConfiguratorProxyClient();

        Windows::Foundation::IAsyncOperation<IResponse^>^ SendCommandAsync(IRequest^ command);
        IResponse^ SendCommand(IRequest^ command);

        __int64 Initialize();

    private:
        handle_t hRpcBinding;
        PCONTEXT_HANDLE_TYPE phContext;
    };
}
