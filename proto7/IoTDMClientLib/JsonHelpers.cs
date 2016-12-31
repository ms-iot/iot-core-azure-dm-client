using System;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System.Collections.Generic;
using System.Diagnostics;

// ToDo: Revisit name space
// ToDo: Revisit accessibility of classes.

namespace Microsoft.Devices.Management
{
    class JsonReader
    {
        public Dictionary<string, JProperty> Properties
        {
            get
            {
                return _properties;
            }
        }

        public JsonReader(string jsonString)
        {
            _properties = new Dictionary<string, JProperty>();

            JObject jsonObject = (JObject)JsonConvert.DeserializeObject(jsonString);
            Flatten(_properties, jsonObject);
        }

        public void GetString(string path, out string stringValue)
        {
            stringValue = "";
            JProperty property;
            if (!_properties.TryGetValue(path, out property))
            {
                throw new Exception("Error: missing property: " + path);
            }

            if (property.Value.Type != JTokenType.String)
            {
                throw new Exception("Error: invalid property type: " + path);
            }

            stringValue = (string)property.Value;
        }

        public void GetBool(string path, out bool boolValue)
        {
            boolValue = false;
            JProperty property;
            if (!_properties.TryGetValue(path, out property))
            {
                throw new Exception("Error: missing property: " + path);
            }

            if (property.Value.Type != JTokenType.Boolean)
            {
                throw new Exception("Error: invalid property type: " + path);
            }

            boolValue = (bool)property.Value;
        }

        public void GetLong(string path, out long numberValue)
        {
            numberValue = 0;

            JProperty property;
            if (!_properties.TryGetValue(path, out property))
            {
                throw new Exception("Error: missing property: " + path);
            }

            if (property.Value.Type != JTokenType.Integer)
            {
                throw new Exception("Error: invalid property type: " + path);
            }

            numberValue = (long)property.Value;
        }

        public void GetDate(string path, out DateTime dateTimeValue)
        {
            dateTimeValue = new DateTime();

            JProperty property;
            if (!_properties.TryGetValue(path, out property))
            {
                throw new Exception("Error: missing property: " + path);
            }

            if (property.Value.Type != JTokenType.Date)
            {
                throw new Exception("Error: invalid property type: " + path);
            }

            dateTimeValue = (DateTime)property.Value;
        }

        private void Flatten(Dictionary<string, JProperty> properties, JObject jsonObj)
        {
            foreach (JProperty jsonProp in jsonObj.Children())
            {
                properties[jsonProp.Value.Path] = jsonProp;
                if (jsonProp.Value.Type == JTokenType.Object)
                {
                    Flatten(properties, (JObject)jsonProp.Value);
                }

            }
        }

        Dictionary<string, JProperty> _properties;
    }

    public class JsonWriter
    {
        public JsonWriter()
        {
            _jsonRoot = new JObject();
        }

        public JProperty GetOrCreate(string path)
        {
            string[] tokens = path.Split('.');

            JObject parent = _jsonRoot;
            for (int i = 0; i < tokens.Length; ++i)
            {
                JProperty child = FindChild(parent, tokens[i]);
                if (child == null)
                {
                    child = new JProperty(tokens[i], null);
                    parent.Add(child);

                    if (i == tokens.Length - 1)
                    {
                        // Time to set
                        return child;
                    }
                    else
                    {
                        // Create a new one and move to it...
                        JObject newObject = new JObject();
                        child.Value = newObject;
                        parent = newObject;
                    }
                }
                else
                {
                    if (i == tokens.Length - 1)
                    {
                        // Time to set
                        return child;
                    }
                    else
                    {
                        // Move to it...
                        parent = (JObject)child.Value;
                    }
                }
            }

            Debug.Assert(false);
            return null;
        }

        public string JsonString
        {
            get
            {
                return _jsonRoot.ToString();
            }
        }

        public JObject Root
        {
            get
            {
                return _jsonRoot;
            }
        }

        private JProperty FindChild(JObject jsonObject, string name)
        {
            foreach (JProperty child in jsonObject.Children())
            {
                if (child.Name == name)
                {
                    return child;
                }
            }
            return null;
        }

        JObject _jsonRoot;
    }
}
