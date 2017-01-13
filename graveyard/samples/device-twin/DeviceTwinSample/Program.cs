using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using Microsoft.Azure.Devices;

namespace DeviceTwinSample
{
    class Temperature
    {
        public int Value;
        public string Units;
    }
    class WindSpeed
    {
        public int Value;
        public string Units;
    }

    class MyDeviceProperties
    {
        public Temperature Temperature;
        public int Humidity;
        public WindSpeed WindSpeed;
    }

    class Program
    {
        // TODO: Change to your hub connection string
        static string iothubConnectionString = "HostName=test10.private.azure-devices-int.net;SharedAccessKeyName=iothubowner;SharedAccessKey=ta3PYCP6p2z2Mot94tQ8o35FATIGqSAME73jd0iTz4o=";

        static void Main(string[] args)
        {
            RegistryManager rm = RegistryManager.CreateFromConnectionString(iothubConnectionString);

            DeviceTwinCRUDSample(rm).Wait();
            DeviceTwinQuerySample(rm).Wait();
        }

        static async Task DeviceTwinCRUDSample(RegistryManager rm)
        {
            //
            //  ADD EMPTY DEVICE
            //
            string deviceId = Guid.NewGuid().ToString();
            var device = await rm.AddDeviceAsync(new Device(deviceId));

            //
            //  UPDATE TWIN
            //
            var deviceTwin = await rm.GetDeviceTwinAsync(deviceId);
            dynamic desiredProperties = deviceTwin.DesiredProperties;
            dynamic tags = new DeviceTwinCollection();  // NOTE: BUG: This should not be needed

            // Typeless
            desiredProperties.Humidity = 100;
            desiredProperties.Temperature = new { };
            desiredProperties.Temperature.Value = 200;
            desiredProperties.Temperature.Units = "F";
            desiredProperties.WindSpeed = new { };
            desiredProperties.WindSpeed.Value = 100;   // Add new 
            desiredProperties.WindSpeed.Units = "MPH"; // Add new
            tags.Location = "Seattle";

            deviceTwin = await rm.UpdateDeviceTwinAsync(deviceId, deviceTwin.DesiredProperties, deviceTwin.Tags, deviceTwin.ETag);

            // Dictionary
            desiredProperties["Humidity"] = 100;
            desiredProperties["Temperature"]["Value"] = 100;
            desiredProperties["Temperature"]["Units"] = "C";
            desiredProperties["WindSpeed"]["Value"] = 50;
            desiredProperties["WindSpeed"]["Units"] = "KPH";
            tags["Location"] = "Seattle";

            deviceTwin = await rm.UpdateDeviceTwinAsync(deviceId, deviceTwin.DesiredProperties, deviceTwin.Tags, deviceTwin.ETag);

            // Typed Property (update)
            desiredProperties = deviceTwin.DesiredProperties;
            Temperature temperature = desiredProperties.Temperature.ConvertTo<Temperature>();
            temperature.Value = 200;
            desiredProperties.Temperature.ConvertFrom<Temperature>(temperature);

            deviceTwin = await rm.UpdateDeviceTwinAsync(deviceId, deviceTwin.DesiredProperties, deviceTwin.Tags, deviceTwin.ETag);

            // Typed Property (add)
            WindSpeed ws = new WindSpeed() { Value = 100, Units = "MPH" };
            desiredProperties.WindSpeed.ConvertFrom<WindSpeed>(ws);

            deviceTwin = await rm.UpdateDeviceTwinAsync(deviceId, deviceTwin.DesiredProperties, deviceTwin.Tags, deviceTwin.ETag);

            // Typed Properties (update property)
            MyDeviceProperties myDeviceProperties = desiredProperties.ConvertTo<MyDeviceProperties>();
            myDeviceProperties.Temperature.Value = 200;
            desiredProperties.ConvertFrom<MyDeviceProperties>(myDeviceProperties);

            deviceTwin = await rm.UpdateDeviceTwinAsync(deviceId, deviceTwin.DesiredProperties, deviceTwin.Tags, deviceTwin.ETag);

            // Typed Properties (add property)
            myDeviceProperties = desiredProperties.ConvertTo<MyDeviceProperties>();
            myDeviceProperties.WindSpeed = new WindSpeed() { Value = 100, Units = "MPH" };
            desiredProperties.ConvertFrom<MyDeviceProperties>(myDeviceProperties);

            deviceTwin = await rm.UpdateDeviceTwinAsync(deviceId, deviceTwin.DesiredProperties, deviceTwin.Tags, deviceTwin.ETag);

            // NOTE: TO BE ADDED: Update json and call deviceTwin = new DeviceTwin.FromJson(json);

            //
            //  GET TWIN STATE
            //
            deviceTwin = await rm.GetDeviceTwinAsync(deviceId);
            desiredProperties = deviceTwin.DesiredProperties;

            // Typeless
            Console.WriteLine(desiredProperties.Humidity + " - " + desiredProperties.Humidity.GetMetadata().LastUpdated);
            Console.WriteLine(desiredProperties.Temperature.Value + " - " + desiredProperties.Temperature.Value.GetMetadata().LastUpdated);
            Console.WriteLine(desiredProperties.Temperature.Units + " - " + desiredProperties.Temperature.Units.GetMetadata().LastUpdated);
            Console.WriteLine(tags.Location);

            // Dictionary
            Console.WriteLine(desiredProperties["Humidity"] + " - " + desiredProperties["Humidity"]["$lastUpdated"]);
            Console.WriteLine(desiredProperties["Temperature"]["Value"] + " - " + desiredProperties["Temperature"]["Value"]["$lastUpdated"]);
            Console.WriteLine(desiredProperties["Temperature"]["Units"] + " - " + desiredProperties["Temperature"]["Units"]["$lastUpdated"]);
            Console.WriteLine(tags["Location"]);

            // Enumerate Dictionary
            foreach (var property in desiredProperties)
            {
                Console.WriteLine(property.Key + " - " + property.Value);
            }

            foreach (var tag in tags)
            {
                Console.WriteLine(tag.Key + " - " + tag.Value);
            }

            // Typed Property
            temperature = desiredProperties.Temperature.ConvertTo<Temperature>();
            // NOTE: Typeless or dictionary interface is used to get metadata
            Console.WriteLine(temperature.Value + " - " + desiredProperties.Temperature.Value.GetMetadata().LastUpdated);
            Console.WriteLine(temperature.Units + " - " + desiredProperties.Temperature.Units.GetMetadata().LastUpdated);

            // Typed Properties
            MyDeviceProperties myDevice = desiredProperties.ConvertTo<MyDeviceProperties>(); ;
            Console.WriteLine(myDevice.Humidity + " - " + desiredProperties.Humidity.GetMetadata().LastUpdated);
            Console.WriteLine(myDevice.Temperature.Value + " - " + desiredProperties.Temperature.Value.GetMetadata().LastUpdated);
            Console.WriteLine(myDevice.Temperature.Units + " - " + desiredProperties.Temperature.Units.GetMetadata().LastUpdated);

            // JSON
            // NOTE: Missing method: Console.WriteLine(deviceTwin.ToJson());
            deviceTwin = await rm.GetDeviceTwinAsync(deviceId);
            Console.WriteLine(deviceTwin.DesiredProperties.ToJson());

            //
            // REPLACE TWIN
            //

            // Typeless
            // NOTE: Dictionary and Typed would work the same as above; Json to be added

            deviceTwin = await rm.GetDeviceTwinAsync(deviceId);
            desiredProperties = new DeviceTwinCollection();

            desiredProperties.Humidity = 100;
            desiredProperties.Temperature = new { };
            desiredProperties.Temperature.Value = 400;
            desiredProperties.Temperature.Units = "F";
            desiredProperties.WindSpeed = new { };
            desiredProperties.WindSpeed.Value = 400;
            desiredProperties.WindSpeed.Units = "MPH";

            deviceTwin = await rm.ReplaceDeviceTwinDesiredPropertiesAsync(deviceId, desiredProperties, deviceTwin.ETag);

            tags = new DeviceTwinCollection();
            tags.Location = "Seattle";
            tags.Department = "Meteorology";

            deviceTwin = await rm.ReplaceDeviceTwinTagsAsync(deviceId, tags, deviceTwin.ETag);

            await rm.RemoveDeviceAsync(deviceId);
        }

        static async Task DeviceTwinQuerySample(RegistryManager rm)
        {
            const int maxDeviceCount = 10;

            await AddDevicesAsync(rm, maxDeviceCount);

            string sqlQuery = "SELECT * FROM devices WHERE desired.Temperature > 50";
            var devices = await rm.QueryDevicesAsync(sqlQuery);
            foreach (var device in devices.Result)
            {
                Console.WriteLine(device.Id + " - " + device.DesiredProperties["Temperature"]);
            }

            sqlQuery = "SELECT * FROM devices WHERE desired.Temperature IN [50,60,70,80,90]";
            devices = await rm.QueryDevicesAsync(sqlQuery);
            foreach (var device in devices.Result)
            {
                Console.WriteLine(device.Id + " - " + device.DesiredProperties["Temperature"]);
            }

            sqlQuery = "SELECT * FROM devices WHERE desired.Temperature > 60 AND (desired.Humidity < 45 OR desired.Humidity > 75)";
            devices = await rm.QueryDevicesAsync(sqlQuery);
            foreach (var device in devices.Result)
            {
                Console.WriteLine(device.Id + " - " + device.DesiredProperties["Temperature"]);
            }

            sqlQuery = "SELECT desired.Manufacturer, AVG(desired.Temperature) AS avgTemperature FROM devices GROUP BY desired.Manufacturer";
            devices = await rm.QueryDevicesAsync(sqlQuery);
            foreach (var deviceAggregate in devices.AggregateResult)
            {
                Console.WriteLine(deviceAggregate["Manufacturer"] + " - " + deviceAggregate["avgTemperature"]);
            }
        }

        static async Task<List<DeviceTwin>> AddDevicesAsync(RegistryManager rm, int count)
        {
            // We can add Custom or System properties as they need to be coming from device
            var random = new Random();
            var devices = new List<DeviceTwin>();
            for (int i = 0; i < count; i++)
            {
                var d = new Device(Guid.NewGuid().ToString());
                await rm.AddDeviceAsync(d);

                DeviceTwin dt = await rm.GetDeviceTwinAsync(d.Id);

                var tagsDelta = new DeviceTwinCollection();
                tagsDelta["Building"] = 109;
                tagsDelta["Version"] = "Ver" + random.Next(0, 2);
                tagsDelta["Floor"] = "Floor" + random.Next(1, 5);

                var desiredPropertiesDelta = new DeviceTwinCollection();
                desiredPropertiesDelta["Temperature"] = random.Next(20, 100);
                desiredPropertiesDelta["Humidity"] = random.Next(35, 95);
                desiredPropertiesDelta["Manufacturer"] = random.Next(0, 2) == 0 ? "ABC" : "XYZ";

                dt = await rm.UpdateDeviceTwinAsync(d.Id, desiredPropertiesDelta, tagsDelta, dt.ETag);
                devices.Add(dt);
            }

            return devices;
        }
    }
}


