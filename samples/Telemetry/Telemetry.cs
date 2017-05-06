using System;
using Microsoft.Azure.Devices.Client;
using Newtonsoft.Json;
using System.Diagnostics;
using System.Runtime.Serialization;
using System.Text;
using System.Threading.Tasks;
using Windows.UI.Popups;
using Windows.Devices.Geolocation;
using Windows.ApplicationModel.Core;
using Windows.UI.Core;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls.Primitives;

namespace IoTDMBackground
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed class Telemetry
    {
        [DataContract]
        internal class DeviceProperties
        {
            [DataMember]
            internal string DeviceID;

            [DataMember]
            internal bool HubEnabledState;

            [DataMember]
            internal string CreatedTime;

            [DataMember]
            internal string DeviceState;

            [DataMember]
            internal string UpdatedTime;

            [DataMember]
            internal string Manufacturer;

            [DataMember]
            internal string ModelNumber;

            [DataMember]
            internal string SerialNumber;

            [DataMember]
            internal string FirmwareVersion;

            [DataMember]
            internal string Platform;

            [DataMember]
            internal string Processor;

            [DataMember]
            internal string InstalledRAM;

            [DataMember]
            internal double Latitude;

            [DataMember]
            internal double Longitude;

        }

        [DataContract]
        internal class CommandParameter
        {
            [DataMember]
            internal string Name;

            [DataMember]
            internal string Type;
        }

        [DataContract]
        internal class Command
        {
            [DataMember]
            internal string Name;

            [DataMember]
            internal CommandParameter[] Parameters;
        }

        [DataContract]
        internal class TelemetryType
        {
            [DataMember]
            internal string Name;

            [DataMember]
            internal string DisplayName;

            [DataMember]
            internal string Type;
        }

        [DataContract]
        internal class DigitalSignDevice
        {
            [DataMember]
            internal DeviceProperties DeviceProperties;

            [DataMember]
            internal Command[] Commands;

            [DataMember]
            internal bool IsSimulatedDevice;

            [DataMember]
            internal TelemetryType[] Telemetry;

            [DataMember]
            internal string Version;

            [DataMember]
            internal string ObjectType;
        }

        [DataContract]
        internal class TelemetryData
        {
            [DataMember]
            internal string DeviceId;

            [DataMember]
            internal double CPUTemperature;

            [DataMember]
            internal double AmbientTemperature;

            [DataMember]
            internal double Proximity;

            [DataMember]
            internal double AmbientNoise;
        }

        private string deviceId;

        private bool SendDataToAzureIoTHub = false;
        private double Temperature = 50;
        private double Humidity = 50;
        private double ExternalTemperature = 50;
        private Windows.Storage.ApplicationDataContainer localSettings = Windows.Storage.ApplicationData.Current.LocalSettings;

        private DeviceClient deviceClient;

        // CPU temp is a flat line
        private WaveGenerator cpuTempDataGenerator = new WaveGenerator { Min = 33, Max = 33, Period = 10, Deviation = 0 };
        private WaveGenerator ambientTempDataGenerator = new WaveGenerator { Min = 18, Max = 22, Period = 5*60, Deviation = 3 };
        private StepGenerator proximityDataGenerator =
            new StepGenerator
            {
                Steps = new StepGenerator.StepDuration[] {
                                new StepGenerator.StepDuration { Value = 10, Duration = 55 },
                                new StepGenerator.StepDuration { Value = 12, Duration = 20 },
                                new StepGenerator.StepDuration { Value = 7, Duration = 33 },
                },
                Deviation = 3
            };

        private StepGenerator ambientNoiseDataGenerator =
            new StepGenerator
            {
                Steps = new StepGenerator.StepDuration[] {
                    new StepGenerator.StepDuration { Value = 42, Duration = 30 },
                    new StepGenerator.StepDuration { Value = 55, Duration = 1 },
                    new StepGenerator.StepDuration { Value = 56, Duration = 1 },
                    new StepGenerator.StepDuration { Value = 51, Duration = 1 },
                    new StepGenerator.StepDuration { Value = 42, Duration = 40 },
                    new StepGenerator.StepDuration { Value = 49, Duration = 1 },
                    new StepGenerator.StepDuration { Value = 44, Duration = 1 },
                    new StepGenerator.StepDuration { Value = 46, Duration = 1 },
            },
                Deviation = 5
            };

        public Telemetry(string deviceId, DeviceClient deviceClient)
        {
            this.deviceId = deviceId;
            this.deviceClient = deviceClient;
        }

        public async Task StartSendingData()
        {
            SendDataToAzureIoTHub = true;
            await sendDeviceMetaData();
            var fireAndForget = Task.Run(SendDataToAzure);
        }

        private byte[] Serialize(object obj)
        {
            string json = JsonConvert.SerializeObject(obj);
            return Encoding.UTF8.GetBytes(json);

        }

        private dynamic DeSerialize(byte[] data)
        {
            string text = Encoding.UTF8.GetString(data);
            return JsonConvert.DeserializeObject(text);
        }

        private async Task sendDeviceMetaData()
        {
            DeviceProperties device = new DeviceProperties();
            DigitalSignDevice thermostat = new DigitalSignDevice();

            thermostat.ObjectType = "DeviceInfo";
            thermostat.IsSimulatedDevice = false;
            thermostat.Version = "1.0";

            device.HubEnabledState = true;
            device.DeviceID = deviceId;
            device.Manufacturer = "Microsoft";
            device.ModelNumber = "X";
            device.SerialNumber = "5849735293875";
            device.FirmwareVersion = "10";
            device.Platform = "Windows 10";
            device.Processor = "SnapDragon";
            device.InstalledRAM = "3GB";
            device.DeviceState = "normal";

            Geolocator geolocator = new Geolocator();
            Geoposition pos = await geolocator.GetGeopositionAsync();

            device.Latitude = (float)pos.Coordinate.Point.Position.Latitude;
            device.Longitude = (float)pos.Coordinate.Point.Position.Longitude;

            thermostat.DeviceProperties = device;

            Command TriggerAlarm = new Command();
            TriggerAlarm.Name = "TriggerAlarm";
            CommandParameter param = new CommandParameter();
            param.Name = "Message";
            param.Type = "String";
            TriggerAlarm.Parameters = new CommandParameter[] { param };

            thermostat.Commands = new Command[] { TriggerAlarm };

            thermostat.Telemetry = new TelemetryType[] { new TelemetryType { Name = "CPUTemperature", DisplayName = " CPU Temperature", Type = "double" },
                                                         new TelemetryType { Name = "AmbientTemperature", DisplayName = "Ambient Temperature", Type = "double" },
                                                         new TelemetryType { Name = "Proximity", DisplayName = "Proximity", Type = "double" },
                                                         new TelemetryType { Name = "AmbientNoise", DisplayName = "Ambient Noise", Type = "double" },
                                                       };

            try
            {
                var msg = new Message(Serialize(thermostat));
                if (deviceClient != null)
                {
                    await deviceClient.SendEventAsync(msg);
                }
            }
            catch (System.Exception e)
            {
                Debug.Write("Exception while sending device meta data :\n" + e.Message.ToString());
            }
        }

        private async void sendDeviceTelemetryData()
        {
            TelemetryData data = new TelemetryData();

            data.DeviceId = deviceId;
            data.CPUTemperature = cpuTempDataGenerator.GetNext();
            data.AmbientTemperature = ambientTempDataGenerator.GetNext();
            data.Proximity = proximityDataGenerator.GetNext();
            data.AmbientNoise = ambientNoiseDataGenerator.GetNext();

            try
            {
                var msg = new Message(Serialize(data));
                if (deviceClient != null)
                {
                    await deviceClient.SendEventAsync(msg);
                }
            }
            catch (System.Exception e)
            {
                Debug.Write("Exception while sending device telemetry data :\n" + e.Message.ToString());
            }
            Debug.Write("Sent telemetry data to IoT Suite\nTemperature=" + string.Format("{0:0.00}", Temperature) + "\nHumidity=" + string.Format("{0:0.00}", Humidity));

        }

        private async Task ReceiveDataFromAzure()
        {
            while (true)
            {
                Message message = await deviceClient.ReceiveAsync();
                if (message != null)
                {
                    try
                    {
                        dynamic command = DeSerialize(message.GetBytes());
                        if (command.Name == "TriggerAlarm")
                        {
                            // Received a new message, display it
                            await CoreApplication.MainView.CoreWindow.Dispatcher.RunAsync(CoreDispatcherPriority.Normal,
                            async () =>
                            {
                                var dialogbox = new MessageDialog("Received message from Azure IoT Hub: " + command.Parameters.Message.ToString());
                                await dialogbox.ShowAsync();
                            });
                            // We received the message, indicate IoTHub we treated it
                            await deviceClient.CompleteAsync(message);
                        }
                    }
                    catch
                    {
                        await deviceClient.RejectAsync(message);
                    }
                }
            }
        }

        private async Task SendDataToAzure()
        {
            while (true)
            {
                if (SendDataToAzureIoTHub)
                {
                    sendDeviceTelemetryData();
                }
                await Task.Delay(1000);
            }
        }

        public async void DisconnectFromIoTSuite()
        {
            if (deviceClient != null)
            {
                try
                {
                    await deviceClient.CloseAsync();
                    deviceClient = null;
                }
                catch (Exception e)
                {
                    Debug.Write("Error while trying close the IoT Hub connection: " + e.Message);
                }
            }
        }
    }
}
