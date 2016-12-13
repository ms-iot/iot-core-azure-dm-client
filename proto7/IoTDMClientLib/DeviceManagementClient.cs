using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System;
using System.Diagnostics;
using System.Threading.Tasks;
using System.Collections.Generic;

namespace Microsoft.Devices.Management
{
    // TODO: this type will come from the Azure IoT SDK
    public enum DeviceTwinUpdateState
    {
        Complete,
        Partial,
    }

    // This is the main entry point into DM
    public class DeviceManagementClient
    {
        // Constants
        public const string RebootMethod = "Reboot";

        // device twin property paths
        public const string DesiredRebootSingleProperty = "properties.desired.reboot.singleReboot";
        public const string DesiredRebootDailyProperty = "properties.desired.reboot.dailyReboot";

        public const string ReportedRebootSingleProperty = "properties.reported.reboot.singleReboot";
        public const string ReportedRebootDailyProperty = "properties.reported.reboot.dailyReboot";
        public const string ReportedLastRebootCmdProperty = "properties.reported.reboot.lastRebootCmd";
        public const string ReportedLastRebootProperty = "properties.reported.reboot.lastReboot";

        // Types
        public struct DMMethodResult
        {
            public uint returnCode;
            public string response;
        }

        struct DesiredProperty
        {
            public string path;
            public string value;
        }

        // Ultimately, DeviceManagementClient will take an abstraction over DeviceClient to allow it to 
        // send reported properties. It will never receive using it
        private DeviceManagementClient(IDeviceTwin deviceTwin, IDeviceManagementRequestHandler requestHandler)
        {
            this.requestHandler = requestHandler;
            this.deviceTwin = deviceTwin;
            this.supportedMethods = new Dictionary<string, Func<string, Task<DMMethodResult>>>();
            this.supportedMethods.Add(RebootMethod, HandleRebootAsync);

            this.supportedProperties = new Dictionary<string, DMCommand>();
            this.supportedProperties.Add(DesiredRebootSingleProperty, DMCommand.SetSingleRebootTime);
            this.supportedProperties.Add(ReportedRebootSingleProperty, DMCommand.GetSingleRebootTime);

            this.supportedProperties.Add(DesiredRebootDailyProperty, DMCommand.SetDailyRebootTime);
            this.supportedProperties.Add(ReportedRebootDailyProperty, DMCommand.GetDailyRebootTime);

            this.supportedProperties.Add(ReportedLastRebootCmdProperty, DMCommand.GetLastRebootCmdTime);

            this.supportedProperties.Add(ReportedLastRebootProperty, DMCommand.GetLastRebootTime);
        }

        public static DeviceManagementClient Create(IDeviceTwin deviceTwin, IDeviceManagementRequestHandler requestHandler)
        {
            return new DeviceManagementClient(deviceTwin, requestHandler);
        }

        public bool IsDMMethod(string methodName)
        {
            return supportedMethods.ContainsKey(methodName);
        }

        public async Task<DMMethodResult> InvokeMethodAsync(string methodName, string payload)
        {
            if (!IsDMMethod(methodName))
            {
                throw new ArgumentException("Unknown method name: " + methodName);
            }

            return await supportedMethods[methodName](payload);
        }

        public bool IsDMProperty(string propertyName)
        {
            return supportedProperties.ContainsKey(propertyName);
        }

        public async Task SetPropertyAsync(string path, string valueString)
        {
            if (!IsDMProperty(path))
            {
                throw new ArgumentException("Unknown property name: " + path);
            }

            var request = new DMRequest();
            request.command = supportedProperties[path];
            request.SetData(valueString);

            DMResponse result = await SystemConfiguratorProxy.SendCommandAsync(request);
            if (result.status != 0)
            {
                throw new Exception();
            }
        }

        public async Task<string> GetPropertyAsync(string path)
        {
            var request = new DMRequest();
            request.command = supportedProperties[path];

            DMResponse result = await SystemConfiguratorProxy.SendCommandAsync(request);
            if (result.status != 0)
            {
                throw new Exception();
            }
            return result.GetDataString();
        }

        public void OnDesiredPropertiesChanged(DeviceTwinUpdateState updateState, string desiredPropertiesString)
        {
            // Traverse the tree and build a list of all the paths references...
            List<DesiredProperty> desiredProperties = new List<DesiredProperty>();
            JObject desiredObj = (JObject)JsonConvert.DeserializeObject(desiredPropertiesString);
            ReadObjectProperties("", desiredProperties, desiredObj);

            // Loop and apply the new values...
            foreach (DesiredProperty dp in desiredProperties)
            {
                SetPropertyAsync(dp.path, dp.value);
            }
        }

        //
        // Commands:
        //

        // This command initiates factory reset of the device
        public async Task StartFactoryReset()
        {
            var request = new DMRequest();
            request.command = DMCommand.FactoryReset;

            // Here we might want to set some reported properties:
            // ReportProperties("We're about to start factory reset... If you don't hear from me again, I'm dead");

            DMResponse result = await SystemConfiguratorProxy.SendCommandAsync(request);
            if (result.status != 0)
            {
                throw new Exception();
            }
        }

        public async Task StartSystemReboot()
        {
            SystemRebootRequestResponse rebootAllowed = await requestHandler.IsSystemRebootAllowed();
            if (rebootAllowed != SystemRebootRequestResponse.StartNow)
            {
                // ToDo: What should happen if the the user blocks the restart?
                //       We need to have a policy on when to ask again.
                return;
            }

            var request = new DMRequest();
            request.command = DMCommand.RebootSystem;

            DMResponse result = await SystemConfiguratorProxy.SendCommandAsync(request);
            if (result.status != 0)
            {
                throw new Exception();
            }
        }

        // This command checks if updates are available. 
        // TODO: work out complete protocol (find updates, apply updates etc.)
        public async Task<bool> CheckForUpdatesAsync()
        {
            var request = new DMRequest();
            request.command = DMCommand.CheckUpdates;

            var response = await SystemConfiguratorProxy.SendCommandAsync(request);

            return response.status == 1;    // 1 means "updates available"
        }

        //
        // Private utilities
        //

        // Report property to DT
        private void ReportProperties(string allJson)
        {
            deviceTwin.ReportProperties(allJson);
        }

        private async Task<DMMethodResult> HandleRebootAsync(string request)
        {
            DMMethodResult result = new DMMethodResult();

            try
            {
                await StartSystemReboot();
                result.returnCode = 1;  // success
            }
            catch (Exception)
            {
                // returnCode is already set to 0 to indicate failure.
            }
            return result;
        }

        private static void ReadProperty(string indent, List<DesiredProperty> desiredProperties, JProperty jsonProp)
        {
            indent += "    ";
            JTokenType type = jsonProp.Type;
            Debug.WriteLine(indent + jsonProp.Name + " = ");

            if (jsonProp.Value.Type == JTokenType.Object)
            {
                ReadObjectProperties(indent, desiredProperties, (JObject)jsonProp.Value);
            }
            else
            {
                JValue theValue = (JValue)jsonProp.Value;
                Debug.WriteLine("Path = " + theValue.Path);
                switch (theValue.Type)
                {
                    case JTokenType.String:
                        {
                            string valueString = (string)theValue.Value;
                            Debug.WriteLine(indent + "value = " + valueString);

                            DesiredProperty desiredProperty = new DesiredProperty();
                            desiredProperty.path = theValue.Path;
                            desiredProperty.value = valueString;
                            desiredProperties.Add(desiredProperty);
                        }
                        break;
                    case JTokenType.Date:
                        {
                            System.DateTime dateTime = (System.DateTime)theValue.Value;
                            Debug.WriteLine(indent + "value 1 = " + dateTime.ToString());

                            DesiredProperty desiredProperty = new DesiredProperty();
                            desiredProperty.path = theValue.Path;

                            // Supported format by CSPs: 2016-10-10T17:00:00Z
                            desiredProperty.value = dateTime.Year + "-" +
                                                    dateTime.Month.ToString("00") + "-" +
                                                    dateTime.Day.ToString("00") + "T" +
                                                    dateTime.Hour.ToString("00") + ":" +
                                                    dateTime.Minute.ToString("00") + ":" +
                                                    dateTime.Second.ToString("00") + "Z";
                            Debug.WriteLine(indent + "value 2 = " + desiredProperty.value);
                            desiredProperties.Add(desiredProperty);
                        }
                        break;
                    case JTokenType.Integer:
                        {
                            long valueInt = (long)theValue.Value;
                            Debug.WriteLine(indent + "value = " + valueInt);

                            DesiredProperty desiredProperty = new DesiredProperty();
                            desiredProperty.path = theValue.Path;
                            desiredProperty.value = valueInt.ToString();
                            desiredProperties.Add(desiredProperty);
                        }
                        break;
                    default:
                        {
                            Debug.WriteLine(indent + "value = " + "unknown value type");
                            Debug.Assert(false);
                        }
                        break;
                }
            }
        }

        private static void ReadObjectProperties(string indent, List<DesiredProperty> desiredProperties, JObject jsonObj)
        {
            indent += "    ";
            foreach (JProperty child in jsonObj.Children())
            {
                ReadProperty(indent, desiredProperties, child);
            }
        }

        // Data members
        IDeviceManagementRequestHandler requestHandler;
        IDeviceTwin deviceTwin;
        Dictionary<string, Func<string, Task<DMMethodResult>>> supportedMethods;
        Dictionary<string, DMCommand> supportedProperties;
    }

}
