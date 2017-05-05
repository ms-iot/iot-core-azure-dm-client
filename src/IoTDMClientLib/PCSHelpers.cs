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
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using Microsoft.Azure.Devices.Client;
using Microsoft.Azure.Devices.Shared;
using System;
using System.Diagnostics;
using System.Text;
using System.Threading.Tasks;

namespace Microsoft.Devices.Management.PCS
{
    static class DeviceModelConstants
    {
        public const string OBJECT_TYPE = "ObjectType";
        public const string VERSION = "Version";
        public const string IS_SIMULATED_DEVICE = "IsSimulatedDevice";
        public const string DEVICE_PROPERTIES = "DeviceProperties";
        public const string COMMANDS = "Commands";
        public const string COMMAND_HISTORY = "CommandHistory";
    }

    static class DevicePropertiesConstants
    {
        public const string DEVICE_ID = "DeviceID";
        public const string DEVICE_STATE = "DeviceState";
        public const string HUB_ENABLED_STATE = "HubEnabledState";
        public const string CREATED_TIME = "CreatedTime";
        public const string UPDATED_TIME = "UpdatedTime";
    }

    static class MessageConstants
    {
        public const string LEVEL = "level";
        public const string LEVEL_NORMAL = "normal";
    }

    public class PCSHelpers
    {
        private static JObject BuildDeviceStructure(string deviceId, bool isSimulated)
        {
            JObject device = new JObject();

            JObject deviceProps = new JObject();
            deviceProps.Add(DevicePropertiesConstants.DEVICE_ID, deviceId);
            deviceProps.Add(DevicePropertiesConstants.HUB_ENABLED_STATE, true);
            deviceProps.Add(DevicePropertiesConstants.CREATED_TIME, DateTime.UtcNow);
            deviceProps.Add(DevicePropertiesConstants.DEVICE_STATE, "normal");
            deviceProps.Add(DevicePropertiesConstants.UPDATED_TIME, null);

            device.Add(DeviceModelConstants.DEVICE_PROPERTIES, deviceProps);
            device.Add(DeviceModelConstants.COMMANDS, new JArray());
            device.Add(DeviceModelConstants.COMMAND_HISTORY, new JArray());
            device.Add(DeviceModelConstants.IS_SIMULATED_DEVICE, isSimulated);

            device.Add(DeviceModelConstants.OBJECT_TYPE, "DeviceInfo");
            device.Add(DeviceModelConstants.VERSION, "1.0");

            return device;
        }

        public static async Task RegisterMethods(DeviceClient deviceClient, string deviceId)
        {
            Debug.WriteLine("Registering " + deviceId + " methods with PCS...");

            JObject device = BuildDeviceStructure(deviceId, false);

            string messageString = device.ToString();
            Debug.WriteLine("Sending message: ", messageString);

            var message = new Microsoft.Azure.Devices.Client.Message(Encoding.ASCII.GetBytes(messageString));
            message.Properties.Add(MessageConstants.LEVEL, MessageConstants.LEVEL_NORMAL);

            await deviceClient.SendEventAsync(message);
        }
        
    }

}