using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Microsoft.Devices.Management
{
    internal interface ISystemConfiguratorProxy
    {
        Task<Message.IResponse> SendCommandAsync(Message.IRequest command);
        Message.IResponse SendCommand(Message.IRequest command);
    }
}
