using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;

using Microsoft.Azure.Devices;
using Microsoft.Azure.Devices.Common;

namespace IoTHubManager
{
    class Program
    {
        class CommandLineException : Exception
        {
            public CommandLineException(string message) : base(message) { }
        }

        class Logger
        {
            public enum LogLevel
            {
                Minimal,
                Normal,
                Verbose,
            }

            public static LogLevel Level = LogLevel.Minimal;

            public static void Log(LogLevel level, string message)
            {
                if (level > Logger.Level)
                {
                    return;
                }
                Console.WriteLine(message);
            }
        }

        class CommandLine
        {
            public Logger.LogLevel LogLevel { get; private set; }
            public string OwnerConnectionString { get; private set; }
            public string DeviceName { get; private set; }

            public static void ShowUsage()
            {
                Logger.Log(Logger.LogLevel.Minimal, "");
                Logger.Log(Logger.LogLevel.Minimal, "Example:");
                Logger.Log(Logger.LogLevel.Minimal, "    IoTHubManager.exe -OwnerConnectionString:<owner connection string> -CreateDevice:<name>");
                Logger.Log(Logger.LogLevel.Minimal, "");
            }

            public static CommandLine Parse(string[] args)
            {
                if (args.Length < 2)
                {
                    throw new CommandLineException("Error: Too few parameters.");
                }

                CommandLine commandLine = new CommandLine();
                commandLine.LogLevel = Logger.LogLevel.Minimal;

                foreach (string arg in args)
                {
                    string[] comps = arg.Split(':');
                    if (comps.Length != 2)
                    {
                        throw new CommandLineException("Error: Invalid switch: '" + arg + "'. Missing :");
                    }
                    if (comps[0].Equals("-OwnerConnectionString", StringComparison.OrdinalIgnoreCase))
                    {
                        commandLine.OwnerConnectionString = comps[1];
                    }
                    else if (comps[0].Equals("-CreateDevice", StringComparison.OrdinalIgnoreCase))
                    {
                        commandLine.DeviceName = comps[1];
                    }
                    else if (comps[0].Equals("-LogLevel", StringComparison.OrdinalIgnoreCase))
                    {
                        if (comps[1].Equals("minimal", StringComparison.OrdinalIgnoreCase))
                        {
                            commandLine.LogLevel = Logger.LogLevel.Minimal;
                        }
                        else if (comps[1].Equals("normal", StringComparison.OrdinalIgnoreCase))
                        {
                            commandLine.LogLevel = Logger.LogLevel.Normal;
                        }
                        else if (comps[1].Equals("verbose", StringComparison.OrdinalIgnoreCase))
                        {
                            commandLine.LogLevel = Logger.LogLevel.Verbose;
                        }
                        else
                        {
                            throw new CommandLineException("Error: Invalid switch value: '" + comps[1]);
                        }
                    }
                }

                if (String.IsNullOrEmpty(commandLine.OwnerConnectionString) || String.IsNullOrEmpty(commandLine.DeviceName))
                {
                    throw new CommandLineException("Missing required switches.");
                }

                return commandLine;
            }
        }

        class OwnerConnectionString
        {
            public string HostName { get; set; }
            public string SharedAccessKeyName { get; set; }
            public string SharedAccessKey { get; set; }

            public static OwnerConnectionString Parse(string connectionString)
            {
                OwnerConnectionString ownerConnectionString = new OwnerConnectionString();

                string[] comps = connectionString.Split(';');
                if (comps.Length != 3)
                {
                    throw new Exception("Error: invalid owner connection string format!");
                }
                foreach(string comp in comps)
                {
                    int index = comp.IndexOf('=');
                    if (index == -1)
                    {
                        throw new Exception("Error: invalid token in owner connection string!");
                    }

                    string name = comp.Substring(0, index);
                    string value = index + 1 < comp.Length ? comp.Substring(index + 1) : "";

                    if (name.Equals("HostName", StringComparison.OrdinalIgnoreCase))
                    {
                        ownerConnectionString.HostName = value;
                    }
                    else if (name.Equals("SharedAccessKeyName", StringComparison.OrdinalIgnoreCase))
                    {
                        ownerConnectionString.SharedAccessKeyName = value;
                    }
                    else if(name.Equals("SharedAccessKey", StringComparison.OrdinalIgnoreCase))
                    {
                        ownerConnectionString.SharedAccessKey = value;
                    }
                }

                return ownerConnectionString;
            }
        }

        static async Task<IEnumerable<string>> ListDevices(RegistryManager registryManager)
        {
            Logger.Log(Logger.LogLevel.Verbose, "Device List:");
            IEnumerable<Device> deviceIds = await registryManager.GetDevicesAsync(100);
            List<String> deviceNames = new List<string>();
            foreach (var deviceId in deviceIds)
            {
                Logger.Log(Logger.LogLevel.Verbose, "-" + deviceId.Id);
                deviceNames.Add(deviceId.Id);
            }
            return deviceNames;
        }

        static async Task<Device> CreateDevice(RegistryManager registryManager, string hostName, string deviceName)
        {
            string primaryKey = CryptoKeyGenerator.GenerateKey(32);
            string secondaryKey = CryptoKeyGenerator.GenerateKey(32);
            Device device = new Device(deviceName);
            device.Authentication = new AuthenticationMechanism();
            device.Authentication.SymmetricKey.PrimaryKey = primaryKey;
            device.Authentication.SymmetricKey.SecondaryKey = secondaryKey;
            await registryManager.AddDeviceAsync(device);

            string deviceConnectionString = "HostName=" + hostName + ";DeviceId=" + deviceName + ";SharedAccessKey=" + primaryKey;
            Logger.Log(Logger.LogLevel.Minimal, "ConnectionString=" + deviceConnectionString);

            return device;
        }

        static string GenerateUniqueName(IEnumerable<string> names, string baseName)
        {
            string name = baseName;
            const uint maxCounter = 1000;
            uint counter = 0;
            while (names.Any<string>(n => n == name) && counter < maxCounter)
            {
                name = baseName + String.Format("{0,0:D3}", counter);
                ++counter;
            }
            if (counter == maxCounter)
            {
                throw new Exception("Too many test devices with the same base name!");
            }
            return name;
        }

        static async Task DoWork(CommandLine commandLine)
        {
            OwnerConnectionString ownerConnectionString = OwnerConnectionString.Parse(commandLine.OwnerConnectionString);
            RegistryManager registryManager = RegistryManager.CreateFromConnectionString(commandLine.OwnerConnectionString);
            IEnumerable<string> deviceNames = await ListDevices(registryManager);
            await CreateDevice(registryManager, ownerConnectionString.HostName, GenerateUniqueName(deviceNames, commandLine.DeviceName));
            await ListDevices(registryManager);
        }

        static int Main(string[] args)
        {
            CommandLine commandLine = null;
            try
            {
                commandLine = CommandLine.Parse(args);
                Logger.Level = commandLine.LogLevel;
            }
            catch(CommandLineException e)
            {
                Logger.Log(Logger.LogLevel.Minimal, e.Message);
                CommandLine.ShowUsage();
                return 1;
            }

            Logger.Log(Logger.LogLevel.Normal, "Command line parameters ok.");

            try
            {
                DoWork(commandLine).Wait(); ;
            }
            catch (Exception e)
            {
                Logger.Log(Logger.LogLevel.Minimal, "Exception caught:");
                while (e != null)
                {
                    Logger.Log(Logger.LogLevel.Minimal, e.Message);
                    e = e.InnerException;
                }
                return 2;
            }

            return 0;
        }
    }
}
