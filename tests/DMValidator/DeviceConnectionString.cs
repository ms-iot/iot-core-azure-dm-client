/*
Copyright 2018 Microsoft
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

using System.Collections.Generic;

namespace DMValidator
{
    class DeviceConnectionString
    {
        public const string HostName = "HostName";
        public const string DeviceId = "DeviceId";
        public const string SharedAccessKey = "SharedAccessKey";

        public DeviceConnectionString(string connectionString)
        {
            _properties = new Dictionary<string, string>();

            ConnectionString = connectionString;
            string[] parts = connectionString.Split(';');
            foreach (string part in parts)
            {
                int index = part.IndexOf('=');
                if (-1 == index)
                {
                    continue;
                }
                string propertyName = part.Substring(0, index);
                string propertyValue = part.Substring(index + 1);
                _properties[propertyName] = propertyValue;
            }
        }

        public string ConnectionString { get; set; }
        public string Property(string propertyName)
        {
            return _properties[propertyName];
        }

        private Dictionary<string, string> _properties;
    }
}
