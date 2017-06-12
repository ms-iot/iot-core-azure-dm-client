using DMDataContract;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace DMDashboard.Wifi
{
    public class WifiProfileConfiguration
    {
        public string Name { get; set; }
        public string Profile { get; set; }
        public bool Uninstall { get; set; }
        public bool DisableInternetConnectivityChecks { get; set; }

        public string ToJsonString()
        {
            var profileName = this.Name;
            StringBuilder sb = new StringBuilder($"\"{profileName}\": ");
            if (this.Uninstall) sb.Append("\"uninstall\"");
            else
            {
                var disable = DisableInternetConnectivityChecks ? "true" : "false";
                sb.Append($"{{\"profile\": \"{Profile}\",\n\"disableInternetConnectivityChecks\": {disable}}}");
            };
            return sb.ToString();
        }

        public static string ToJsonString(ICollection<WifiProfileConfiguration> profiles, string applyPropertiesValue, string reportPropertiesValue)
        {
            StringBuilder sb = new StringBuilder("\"wifi\": ");

            bool reporting = DMJSonConstants.YesString.Equals(reportPropertiesValue);
            bool applying = DMJSonConstants.YesString.Equals(applyPropertiesValue);

            if (!applying && !reporting) sb.Append($"\"{DMJSonConstants.NoApplyNoReportString}\"\n");
            else if (!applying && reporting) sb.Append($"\"{DMJSonConstants.NoApplyYesReportString}\"\n");
            else
            {
                bool first = true;
                sb.Append($"{{\n\"{DMJSonConstants.ApplyPropertiesString}\": {{");
                foreach (var profile in profiles)
                {
                    if (!first)
                    {
                        sb.Append(",");
                    }
                    sb.Append("\n");
                    first = false;

                    sb.Append(profile.ToJsonString());
                }
                sb.Append("\n},\n");
                if (reporting) sb.Append($"\"{DMJSonConstants.ReportPropertiesString}\": \"{DMJSonConstants.YesString}\"\n}}");
                else sb.Append($"\"{DMJSonConstants.ReportPropertiesString}\": \"{DMJSonConstants.NoString}\"\n}}");
            }

            return sb.ToString();
        }

        public static void ReadReportedFromJson(JToken token, ICollection<WifiProfileConfiguration> profiles)
        {
            if (!(token is JObject))
            {
                MessageBox.Show("Error: invalid apps node json format!");
                return;
            }
            profiles.Clear();

            JObject root = (JObject)token;
            foreach (JToken p in root.Children())
            {
                if (!(p is JProperty))
                {
                    continue;
                }
                JProperty property = (JProperty)p;
                string profileName = property.Name;
                profiles.Add(new WifiProfileConfiguration() { Name = profileName });
            }
        }
    }


}
