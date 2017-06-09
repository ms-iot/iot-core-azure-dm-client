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
        // TODO: replace after rebase w/ George's code
        public const string ApplyPropertiesString = "applyProperties";
        public const string ReportPropertiesString = "reportProperties";
        public const string YesString = "yes";
        public const string NoString = "no";
        public const string NoApplyYesReportString = "no-apply-yes-report";
        public const string NoApplyNoReportString = "no-apply-no-report";
        public const string NoReportString = "no-report";
        public const string NotFoundString = "<not found>";
        public const string NAString = "n/a";


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

        public static string ToJsonString(ICollection<WifiProfileConfiguration> profiles)
        {
            StringBuilder sb = new StringBuilder("\"wifi\": ");

            if (profiles.Count == 0) sb.Append($"\"{NoApplyNoReportString}\"\n");
            else
            {
                bool first = true;
                sb.Append($"{{\n\"{ApplyPropertiesString}\": {{");
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
                sb.Append($"\"{ReportPropertiesString}\": \"{YesString}\"\n}}");
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
