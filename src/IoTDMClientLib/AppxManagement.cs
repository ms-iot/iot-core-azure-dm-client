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
using Microsoft.Devices.Management;
using Microsoft.Devices.Management.Message;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;
using System.Threading.Tasks;

namespace Microsoft.Devices.Management
{
    class AppReportedState
    {
        public string packageFamilyId;
        public string packageFamilyName;
        public string version;
        public string installDate;
        public Error error;
        public JsonReport report;

        public AppReportedState(string packageFamilyId, string packageFamilyName, string version, string installDate, Error error, JsonReport report)
        {
            this.packageFamilyId = packageFamilyId;
            this.packageFamilyName = packageFamilyName;
            this.version = version;
            this.installDate = installDate;
            this.error = error;
            this.report = report;
        }

        public string ToJson()
        {
            StringBuilder sb = new StringBuilder();
            sb.Append("\"" + packageFamilyId + "\" : ");
            if (report == JsonReport.Report)
            {
                sb.Append("{\n");
                sb.Append("                \"pkgFamilyName\" : \"" + packageFamilyName + "\",\n");
                sb.Append("                \"version\" : \"" + version + "\",\n");
                sb.Append("                \"installDate\" : \"" + installDate + "\",\n");
                if (error != null)
                {
                    sb.Append("                \"errorCode\" : \"" + (error.HResult.ToString()) + "\",\n");
                    sb.Append("                \"errorMessage\" : \"" + (error.Message) + "\"\n");
                }
                else
                {
                    sb.Append("                \"errorCode\" : null,\n");
                    sb.Append("                \"errorMessage\" : null\n");
                }
                sb.Append("            }\n");
            }
            else
            {
                sb.Append("null");
            }

            return sb.ToString();
        }
    }

    enum AppDesiredAction
    {
        Undefined,
        Install,
        Uninstall,
        Query,
        Unreport
    }

    class AppDesiredState
    {
        public string packageFamilyId;
        public string packageFamilyName;
        public AppDesiredAction action;
        public Version version;
        public string appxSource;
        public string depsSources;
        public string certSource;
        public string certStore;
        public Error error;

        public AppDesiredState(string packageFamilyId)
        {
            this.packageFamilyId = packageFamilyId;
            this.action = AppDesiredAction.Undefined;
        }

        public void Dump()
        {
            Debug.WriteLine("- Desired State ------------------------------------------------");
            Debug.WriteLine("packageFamilyId = " + packageFamilyId);
            Debug.WriteLine("packageFamilyName   = " + packageFamilyName);
            Debug.WriteLine("action              = " + action.ToString());
            Debug.WriteLine("version             = " + version?.ToString());
            Debug.WriteLine("appxSource          = " + appxSource);
            Debug.WriteLine("depsSources         = " + depsSources);
            Debug.WriteLine("certSource          = " + certSource);
            Debug.WriteLine("certStore           = " + certStore);
            if (error != null)
            {
                Debug.WriteLine("errorCode           = " + error.HResult);
                Debug.WriteLine("errorMessage        = " + error.Message);
            }
            Debug.WriteLine("----------------------------------------------------------------");
        }
    }

    class AppxManagement
    {
        private static string VersionNotInstalled = "not installed";

        private static void DumpInstalledApps(IDictionary<string, AppInfo> data)
        {
            foreach (KeyValuePair<string, AppInfo> p in data)
            {
                Debug.WriteLine("- Installed App ---------------------------------------------");
                Debug.WriteLine("Key               = " + p.Key);
                Debug.WriteLine("Name              = " + p.Value.Name);
                Debug.WriteLine("PackageFamilyName = " + p.Value.PackageFamilyName);
                Debug.WriteLine("AppSource         = " + p.Value.AppSource);
                Debug.WriteLine("InstallDate       = " + p.Value.InstallDate);
                Debug.WriteLine("InstallLocation   = " + p.Value.InstallLocation);
                Debug.WriteLine("Architecture      = " + p.Value.Architecture);
                Debug.WriteLine("Publisher         = " + p.Value.Publisher);
                Debug.WriteLine("Version           = " + p.Value.Version);
                Debug.WriteLine("Users             = " + p.Value.Users);
                Debug.WriteLine("-------------------------------------------------------------");
            }
        }

        private static async Task ReportAppStatus(DeviceManagementClient client, AppReportedState appReportedState)
        {
            StringBuilder sb = new StringBuilder();
            sb.Append("{\n");
            sb.Append("    \"management\" : {\n");
            sb.Append("        \"apps\" : {\n");
            sb.Append("            " + appReportedState.ToJson());
            sb.Append("        }\n");
            sb.Append("    }\n");
            sb.Append("}\n");

            Debug.WriteLine("Report:\n" + sb.ToString());

            Dictionary<string, object> collection = new Dictionary<string, object>();
            collection["microsoft"] = JsonConvert.DeserializeObject(sb.ToString());
            await client.DeviceTwin.ReportProperties(collection);
        }

        private static IEnumerable<AppDesiredState> JsonToDesiredStates(JObject appsNode)
        {
            List<AppDesiredState> desiredStates = new List<AppDesiredState>();

            foreach (JProperty property in appsNode.Properties())
            {
                if (String.IsNullOrEmpty(property.Name))
                {
                    continue;
                }

                AppDesiredState desiredState = new AppDesiredState(property.Name);

                if (property.Value is JObject)
                {
                    JObject appPropertiesObject = (JObject)property.Value;

                    foreach (JProperty appProperty in appPropertiesObject.Properties())
                    {
                        // We expect non-null non-object values.
                        if (appProperty == null || appProperty.Value == null || !(appProperty.Value is JValue))
                        {
                            continue;
                        }

                        if (appProperty.Name == "pkgFamilyName")
                        {
                            desiredState.packageFamilyName = appProperty.Value.ToString();
                        }
                        else if (appProperty.Name == "version")
                        {
                            string value = appProperty.Value.ToString();
                            if (value == "?")
                            {
                                desiredState.action = AppDesiredAction.Query;
                            }
                            else if (value == VersionNotInstalled)
                            {
                                desiredState.action = AppDesiredAction.Uninstall;
                            }
                            else
                            {
                                Version result = null;
                                if (Version.TryParse(value, out result))
                                {
                                    desiredState.action = AppDesiredAction.Install;
                                    desiredState.version = result;
                                }
                                else
                                {
                                    desiredState.action = AppDesiredAction.Undefined;
                                    desiredState.error = new Error(ErrorCodes.INVALID_DESIRED_VERSION, "Invalid desired version format!");
                                }
                            }
                        }
                        else if (appProperty.Name == "appxSource")
                        {
                            desiredState.appxSource = appProperty.Value.ToString();
                        }
                        else if (appProperty.Name == "depsSources")
                        {
                            desiredState.depsSources = appProperty.Value.ToString();
                        }
                        else if (appProperty.Name == "certSource")
                        {
                            desiredState.certSource = appProperty.Value.ToString();
                        }
                        else if (appProperty.Name == "certStore")
                        {
                            desiredState.certStore = appProperty.Value.ToString();
                        }
                    }
                }
                else
                {
                    JValue value = (JValue)property.Value;
                    if (value == null || value.Value == null)
                    {
                        desiredState.action = AppDesiredAction.Unreport;
                    }
                    else
                    {
                        desiredState.action = AppDesiredAction.Undefined;
                        desiredState.error = new Error(ErrorCodes.INVALID_DESIRED_PKG_FAMILY_ID, "Invalid desired package family id value!");
                    }
                }

                desiredState.Dump();
                desiredStates.Add(desiredState);
            }
            return desiredStates;
        }

        private static async Task<AppReportedState> UpdateAppFromStore(AppDesiredState desiredState)
        {
            Debug.WriteLine("Updating " + desiredState.packageFamilyName + " from store");
            return new AppReportedState(
                desiredState.packageFamilyId,
                desiredState.packageFamilyName,
                desiredState.version.ToString(),
                "",
                new Error(ErrorCodes.E_NOTIMPL, "Updating app from the store has not been implemented yet."),
                JsonReport.Report);
        }

        private static async Task UninstallAppAsync(DeviceManagementClient client, AppInfo appInfo, string packageFamilyId, string packageFamilyName)
        {
            AppReportedState reportedState = null;
            try
            {
                // ToDo: We need to handle store and system apps too.
                AppUninstallResponse response = await client.UninstallAppAsync(new AppUninstallRequestData(packageFamilyName, false /*non-store app*/));
                if (response.Status == ResponseStatus.Success)
                {
                    reportedState = new AppReportedState(packageFamilyId,
                                                         packageFamilyName,
                                                         VersionNotInstalled,
                                                         null,   // no install date
                                                         null,   // no error
                                                         JsonReport.Report);
                    await ReportAppStatus(client, reportedState);
                    return;
                }
                else
                {
                    throw new Error(response.data.errorCode, response.data.errorMessage);
                }
            }
            catch(Error e)
            {
                reportedState = new AppReportedState(packageFamilyId,
                                                     packageFamilyName,
                                                     appInfo.Version,
                                                     appInfo.InstallDate,   // install date
                                                     e,
                                                     JsonReport.Report);
            }
            catch (Exception e)
            {
                reportedState = new AppReportedState(packageFamilyId,
                                                     packageFamilyName,
                                                     appInfo.Version,
                                                     appInfo.InstallDate,   // install date
                                                     new Error(e.HResult, e.Message),
                                                     JsonReport.Report);
            }
            await ReportAppStatus(client, reportedState);
            throw new Exception("Failed to uninstall " + packageFamilyName);
        }

        private static async Task InstallAppFromAzureAsync(DeviceManagementClient client, AppInfo currentState, string connectionString, AppDesiredState desiredState)
        {
            // Is this a fresh installation?
            if (currentState == null)
            {
                currentState = new AppInfo();
                currentState.PackageFamilyName = desiredState.packageFamilyName;
                currentState.Version = VersionNotInstalled;
                currentState.InstallDate = "";
            }

            AppReportedState reportedState = null;
            try
            {
                Debug.WriteLine("-> installing " + desiredState.packageFamilyName + " from " + desiredState.appxSource);
                if (String.IsNullOrEmpty(desiredState.appxSource))
                {
                    throw new Error(ErrorCodes.INVALID_DESIRED_APPX_SRC, "Cannot install appx without a source.");
                }

                var requestData = new AppInstallRequestData();
                requestData.PackageFamilyName = desiredState.packageFamilyName;

                // Downloading dependencies...
                Debug.WriteLine(DateTime.Now.ToString("HH:mm:ss") + " Downloading...");
                if (!String.IsNullOrEmpty(desiredState.depsSources))
                {
                    string[] depsSources = desiredState.depsSources.Split(';');

                    for (int i = 0; i < depsSources.Length; ++i)
                    {
                        IoTDMClient.BlobInfo dependencyBlob = IoTDMClient.BlobInfo.BlobInfoFromSource(connectionString, depsSources[i]);
                        Debug.WriteLine(DateTime.Now.ToString("HH:mm:ss") + " Downloading " + dependencyBlob.BlobName);

                        var dependencyPath = await dependencyBlob.DownloadToTempAsync(client);
                        Debug.WriteLine(dependencyPath);
                        requestData.Dependencies.Add(dependencyPath);
                    }
                }

                // Downloading certificates...
                Debug.WriteLine(DateTime.Now.ToString("HH:mm:ss") + " Downloading appx certificate...");
                IoTDMClient.BlobInfo certificateBlob = IoTDMClient.BlobInfo.BlobInfoFromSource(connectionString, desiredState.certSource);
                requestData.CertFile = await certificateBlob.DownloadToTempAsync(client);
                requestData.CertStore = desiredState.certStore;

                // Downloading appx...
                Debug.WriteLine(DateTime.Now.ToString("HH:mm:ss") + " Downloading appx...");
                IoTDMClient.BlobInfo appxBlob = IoTDMClient.BlobInfo.BlobInfoFromSource(connectionString, desiredState.appxSource);
                requestData.AppxPath = await appxBlob.DownloadToTempAsync(client);

                // Installing appx...
                Debug.WriteLine(DateTime.Now.ToString("HH:mm:ss") + " Installing appx...");
                AppInstallResponse response = await client.InstallAppAsync(requestData);
                Debug.WriteLine(DateTime.Now.ToString("HH:mm:ss") + " Done installing appx...");

                Error e = null;
                if (desiredState.version.ToString() != response.data.version)
                {
                    if (currentState.Version == response.data.version)
                    {
                        e = new Error(ErrorCodes.INVALID_INSTALLED_APP_VERSION_UNCHANGED, "Installating the supplied appx succeeded, but the new app version is not the desired version, and is the same as the old app version.");
                    }
                    else
                    {
                        e = new Error(ErrorCodes.INVALID_INSTALLED_APP_VERSION_UNEXPECTED, "Installating the supplied appx succeeded, but the new app version is not the desired version.");
                    }
                }

                if (response.Status == ResponseStatus.Success)
                {
                    reportedState = new AppReportedState(desiredState.packageFamilyId,
                                                         desiredState.packageFamilyName,
                                                         response.data.version,
                                                         response.data.installDate,
                                                         e,
                                                         JsonReport.Report);
                    await ReportAppStatus(client, reportedState);
                    return;
                }
                else
                {
                    throw new Error(response.data.errorCode, response.data.errorMessage);
                }
            }
            catch (Error e)
            {
                reportedState = new AppReportedState(desiredState.packageFamilyId,
                                                     desiredState.packageFamilyName,
                                                     currentState.Version,
                                                     currentState.InstallDate,   // install date
                                                     e,
                                                     JsonReport.Report);
            }
            catch (Exception e)
            {
                reportedState = new AppReportedState(desiredState.packageFamilyId,
                                                     desiredState.packageFamilyName,
                                                     currentState.Version,
                                                     currentState.InstallDate,   // install date
                                                     new Error(e.HResult, e.Message),
                                                     JsonReport.Report);
            }
            await ReportAppStatus(client, reportedState);
            throw new Exception("Failed to install " + desiredState.packageFamilyName);
        }

        private static async Task ApplyAppDesiredState(DeviceManagementClient client, string connectionString, IDictionary<string, AppInfo> installedApps, AppDesiredState desiredState)
        {
            AppInfo installedAppInfo = null;
            Version installedAppVersion = null;

            // Let's get information of the app now... we will use it later.
            foreach (KeyValuePair<string, AppInfo> pair in installedApps)
            {
                AppInfo appInfo = pair.Value;
                if (appInfo.PackageFamilyName == desiredState.packageFamilyName)
                {
                    installedAppInfo = appInfo;
                    installedAppVersion = new Version(installedAppInfo.Version);
                    break;
                }
            }

            switch (desiredState.action)
            {
                case AppDesiredAction.Install:
                    {
                        Debug.WriteLine("Processing install request...");

                        if (installedAppInfo == null)
                        {
                            // It is a new application.
                            Debug.WriteLine("    Can't find an installed version... Installing a fresh copy...");
                            await InstallAppFromAzureAsync(client, installedAppInfo, connectionString, desiredState);
                        }
                        else
                        {
                            // A version of this application is installed.
                            Debug.WriteLine("    Found an installed version...");

                            if (desiredState.version == installedAppVersion)
                            {
                                Debug.WriteLine("        Same version is installed...");

                                AppReportedState appReportedState = new AppReportedState(
                                    desiredState.packageFamilyId,
                                    desiredState.packageFamilyName,
                                    installedAppVersion.ToString(),
                                    installedAppInfo.InstallDate,
                                    null,                 // no error
                                    JsonReport.Report);

                                await ReportAppStatus(client, appReportedState);
                            }
                            else if (desiredState.version > installedAppVersion)
                            {
                                Debug.WriteLine("        Older version is installed...");

                                if (!String.IsNullOrEmpty(desiredState.appxSource))
                                {
                                    await InstallAppFromAzureAsync(client, installedAppInfo, connectionString, desiredState);
                                }
                                else
                                {
                                    await UpdateAppFromStore(desiredState);
                                }
                            }
                            else
                            {
                                // desiredState.version < installedAppVersion
                                Debug.WriteLine("       Newer version is installed...rolling back.");

                                if (String.IsNullOrEmpty(desiredState.appxSource))
                                {
                                    AppReportedState appReportedState = new AppReportedState(
                                        desiredState.packageFamilyId,
                                        desiredState.packageFamilyName,
                                        installedAppVersion.ToString(),
                                        installedAppInfo.InstallDate,
                                        new Error(ErrorCodes.INVALID_DESIRED_APPX_SRC, "Cannot install appx without a source."),
                                        JsonReport.Report);

                                    await ReportAppStatus(client, appReportedState);

                                    throw new Exception("Failed to roll back application version.");
                                }
                                else
                                {
                                    // Note that UninstallAppAsync will throw if it fails - and correctly avoid launching the install...
                                    await UninstallAppAsync(client, installedAppInfo, desiredState.packageFamilyId, desiredState.packageFamilyId);
                                    await InstallAppFromAzureAsync(client, installedAppInfo, connectionString, desiredState);
                                }
                            }
                        }
                    }
                    break;
                case AppDesiredAction.Uninstall:
                    {
                        Debug.WriteLine("Processing uninstall request...");
                        await UninstallAppAsync(client, installedAppInfo, desiredState.packageFamilyId, desiredState.packageFamilyId);
                    }
                    break;
                case AppDesiredAction.Query:
                    {
                        Debug.WriteLine("Processing query request...");

                        AppReportedState appReportedState;
                        if (installedAppInfo == null)
                        {
                            Debug.WriteLine("    Couldn't find an installed version...");

                            appReportedState = new AppReportedState(
                                desiredState.packageFamilyId,
                                desiredState.packageFamilyName,
                                VersionNotInstalled,
                                null,
                                null,
                                JsonReport.Report);
                        }
                        else
                        {
                            Debug.WriteLine("    Found an installed version...");

                            appReportedState = new AppReportedState(
                                desiredState.packageFamilyId,
                                desiredState.packageFamilyName,
                                installedAppInfo.Version,
                                installedAppInfo.InstallDate,
                                null,
                                JsonReport.Report);
                        }
                        await ReportAppStatus(client, appReportedState);
                    }
                    break;
                case AppDesiredAction.Unreport:
                    {
                        Debug.WriteLine("Processing unreport request...");

                        AppReportedState appReportedState = new AppReportedState(
                            desiredState.packageFamilyId,
                            desiredState.packageFamilyName,
                            null,   // no version
                            null,   // no install date
                            null,   // no error
                            JsonReport.Unreport);

                        await ReportAppStatus(client, appReportedState);
                    }
                    break;
                case AppDesiredAction.Undefined:
                    {
                        Debug.WriteLine("Processing unknown request...");

                        AppReportedState appReportedState = new AppReportedState(
                            desiredState.packageFamilyId,
                            desiredState.packageFamilyName,
                            null,   // no version
                            null,   // no install date
                            new Error(ErrorCodes.INVALID_DESIRED_APPX_OPERATION, "Invalid application operation."),
                            JsonReport.Report);

                        await ReportAppStatus(client, appReportedState);
                    }
                    break;
            }
        }

        public static async Task ApplyDesiredAppsConfiguration(DeviceManagementClient client, string connectionString, JObject appsNode)
        {
            IDictionary<string, AppInfo> installedApps = await client.ListAppsAsync();

            DumpInstalledApps(installedApps);

            IEnumerable<AppDesiredState> desiredStates = JsonToDesiredStates(appsNode);
            foreach (AppDesiredState desiredState in desiredStates)
            {
                try
                {
                    await ApplyAppDesiredState(client, connectionString, installedApps, desiredState);
                }
                catch(Exception)
                {
                    // Catch everything here so that we may continue processing other desiredStates.
                    // No reporting to the IoT Hub is needed here because it has already taken place.
                }
            }
        }
    }
}
