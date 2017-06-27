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
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Microsoft.Devices.Management
{
    struct AppListQuery
    {
        public bool store;
        public bool nonStore;
    }

    struct DesiredActions
    {
        public List<AppDesiredState> appDesiredStates;
        public AppListQuery appListQuery;

        public DesiredActions(List<AppDesiredState> states, AppListQuery query)
        {
            this.appDesiredStates = states;
            this.appListQuery = query;
        }
    }

    class AppUtils
    {
        public const string JsonStartUpNone = "none";
        public const string JsonStartUpForeground = "foreground";
        public const string JsonStartUpBackground = "background";

        public static StartUpType StartUpTypeFromString(string value)
        {
            StartUpType startUp = StartUpType.None;
            switch (value)
            {
                case JsonStartUpForeground:
                    startUp = StartUpType.Foreground;
                    break;
                case JsonStartUpBackground:
                    startUp = StartUpType.Background;
                    break;
            }
            return startUp;
        }

        public static string StringFromStartUpType(StartUpType value)
        {
            string startUp = JsonStartUpNone;
            switch (value)
            {
                case StartUpType.Foreground:
                    startUp = JsonStartUpForeground;
                    break;
                case StartUpType.Background:
                    startUp = JsonStartUpBackground;
                    break;
            }
            return startUp;
        }
    }

    class AppReportedState
    {
        public string packageFamilyId;
        public string packageFamilyName;
        public string version;
        public StartUpType startUp;
        public string installDate;
        public Error error;
        public JsonReport report;

        public AppReportedState(string packageFamilyId, string packageFamilyName, string version, StartUpType startUp, string installDate, Error error, JsonReport report)
        {
            this.packageFamilyId = packageFamilyId;
            this.packageFamilyName = packageFamilyName;
            this.version = version;
            this.startUp = startUp;
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
                sb.Append("                \"startUp\" : \"" + AppUtils.StringFromStartUpType(startUp) + "\",\n");
                sb.Append("                \"installDate\" : \"" + installDate + "\",\n");
                if (error != null)
                {
                    sb.Append("                \"errorCode\" : \"" + (error.HResult.ToString()) + "\",\n");
                    sb.Append("                \"errorMessage\" : \"" + (error.Message) + "\"\n");
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
        public Message.StartUpType startUp;
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
            Debug.WriteLine("startUp             = " + startUp.ToString());
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

    class AppxManagement : IClientPropertyHandler, IClientPropertyDependencyHandler
    {
        const string JsonSectionName = "apps";
        string[] JsonSectionDependencyNames = { "externalStorage" };

        private static string VersionNotInstalled = "not installed";

        public AppxManagement(IClientHandlerCallBack callback, ISystemConfiguratorProxy systemConfiguratorProxy, JObject desiredCache)
        {
            this._systemConfiguratorProxy = systemConfiguratorProxy;
            this._callback = callback;
            this._desiredCache = desiredCache;
        }

        // IClientPropertyDependencyHandler
        public string[] PropertySectionDependencyNames
        {
            get
            {
                return JsonSectionDependencyNames; // todo: constant in data contract?
            }
        }

        // IClientPropertyDependencyHandler
        public void OnDesiredPropertyDependencyChange(string section, JObject value)
        {
            if (section.Equals(JsonSectionDependencyNames[0]))
            {
                // externalStorage
                this._connectionString = (string)value.Property("connectionString").Value;
            }
        }

        // IClientPropertyHandler
        public string PropertySectionName
        {
            get
            {
                return JsonSectionName;
            }
        }

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
                Debug.WriteLine("StartUp           = " + p.Value.StartUp);
                Debug.WriteLine("-------------------------------------------------------------");
            }
        }

        private async Task ReportAppStatus(AppReportedState appReportedState)
        {
            StringBuilder sb = new StringBuilder();
            sb.Append("{\n");
            sb.Append("    " + appReportedState.ToJson());
            sb.Append("}\n");

            Debug.WriteLine("Report:\n" + sb.ToString());
            await this._callback.ReportPropertiesAsync(JsonSectionName, (JToken)JsonConvert.DeserializeObject(sb.ToString()));
        }

        private async Task NullifyReported()
        {
            Debug.WriteLine("NullifyReported\n");
            await this._callback.ReportPropertiesAsync(JsonSectionName, new JValue("refreshing"));
        }

        private async Task ReportGeneralError(int errorCode, string errorMessage)
        {
            Debug.WriteLine("ReportGeneralError\n");

            /*
            StringBuilder sb = new StringBuilder();
            sb.Append("{\n");
            sb.Append("  \"generalError\" : {\n");
            sb.Append("    \"code\" : " + errorCode + ",\n");
            sb.Append("    \"message\" : \"" + errorMessage + "\",\n");
            sb.Append("  }\n");
            sb.Append("}\n");
            JObject jObject = (JObject)JsonConvert.DeserializeObject(sb.ToString());
            */
            JObject jObject = new JObject();
            jObject["code"] = errorCode;
            jObject["message"] = errorMessage;
            await this._callback.ReportPropertiesAsync(JsonSectionName, jObject);
        }

        private void ReorderAndValidate(List<AppDesiredState> appDesiredStates, IDictionary<string, AppInfo> installedApps)
        {
            // If we are switching the foreground app from App1 to App2, App2 must be processed first.
            // For example:
            // - Startup is set to App1.
            // - Desired state is:
            //   - App1 : none
            //   - App2 : foreground
            // We have to process App2 first and that will implicitly apply App1 : none.

            // Note that appDesiredStates are not only the just-changed ones - but all the desired states for all apps.
            // This covers the case where the user sets a new foreground app and forgets to set the old one to 'none'.

            AppDesiredState desiredForegroundApp = null;
            var foregroundApps = from state in appDesiredStates
                         where state.startUp == StartUpType.Foreground
                         select state;

            foreach (AppDesiredState state in foregroundApps)
            {
                Debug.WriteLine("Found foreground app: " + state.packageFamilyName);
                desiredForegroundApp = state;
            }

            if (foregroundApps.Count<AppDesiredState>() > 1)
            {
                StringBuilder sb = new StringBuilder();
                foreach(AppDesiredState s in foregroundApps)
                {
                    if (sb.Length > 0)
                    {
                        sb.Append(", ");
                    }
                    sb.Append(s.packageFamilyName);
                }

                throw new Error(ErrorCodes.INVALID_DESIRED_MULTIPLE_FOREGROUND_APPS, "Cannot set more than one application to be the foreground application! Package Family Names = " + sb.ToString());
            }

            // Make sure the foreground application is not being uninstalled.
            if (desiredForegroundApp != null && desiredForegroundApp.startUp == StartUpType.Foreground && desiredForegroundApp.action == AppDesiredAction.Uninstall)
            {
                // This means that no other application has been set to replace the one about to be uninstalled.
                throw new Error(ErrorCodes.INVALID_DESIRED_CONFLICT_UNINSTALL_FOREGROUND_APP, "Cannot configure an application to the foreground application and uninstall it. Package Family Name = " + desiredForegroundApp.packageFamilyName);
            }

            // Make sure that the application being uninstalled is not the foreground app as marked by the system (i.e. installedApps).
            // This is to catch the case where the desired state does not say anything about foreground apps - but the system does.
            // So, we want to make sure the desired state does not conflict with the system state.
            var uninstallForegroundApps = from desiredState in appDesiredStates
                                          join installedState in installedApps on desiredState.packageFamilyName equals installedState.Value.PackageFamilyName
                                          where desiredState.action == AppDesiredAction.Uninstall && installedState.Value.StartUp == StartUpType.Foreground
                                          select desiredState;

            if (uninstallForegroundApps.Count<AppDesiredState>() > 0)
            {
                // There should be at most one application matching since the foreground setting on the system allows only one foreground application.
                string foregroundAppToUninstall = uninstallForegroundApps.First<AppDesiredState>().packageFamilyName;

                // Note that it is okay to uninstall the foreground app if another one is being set as the foreground in the same transaction.
                if (desiredForegroundApp.packageFamilyName == foregroundAppToUninstall)
                {
                    // This means that an application is marked for uninstallation, and 
                    throw new Error(ErrorCodes.INVALID_DESIRED_CONFLICT_UNINSTALL_FOREGROUND_APP, "Cannot uninstall a foreground application. Package Family Name = " + foregroundAppToUninstall);
                }
            }

            if (desiredForegroundApp != null)
            {
                // Move to the front.
                appDesiredStates.Remove(desiredForegroundApp);
                appDesiredStates.Insert(0, desiredForegroundApp);
            }

            Debug.WriteLine("Ordered:");
            foreach (AppDesiredState state in appDesiredStates)
            {
                Debug.WriteLine("App: " + state.packageFamilyName + " " + state.startUp.ToString());
            }
        }

        private AppDesiredState JsonToAppDesiredState(string packageFamilyId, JToken jToken)
        {
            AppDesiredState desiredState = new AppDesiredState(packageFamilyId);

            if (jToken is JObject)
            {
                JObject appPropertiesObject = (JObject)jToken;

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
                    else if (appProperty.Name == "startUp")
                    {
                        desiredState.startUp = AppUtils.StartUpTypeFromString(appProperty.Value.ToString());
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
                JValue value = (JValue)jToken;
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
            return desiredState;
        }

        private DesiredActions JsonToDesiredActions(JObject appsNode)
        {
            List<AppDesiredState> appDesiredStates = new List<AppDesiredState>();
            AppListQuery appListQuery = new AppListQuery();

            foreach (JProperty property in appsNode.Children())
            {
                Debug.WriteLine("Property Name = " + property.Name);
                if (String.IsNullOrEmpty(property.Name))
                {
                    continue;
                }

                if (property.Name == "?")
                {
                    appListQuery = JsonConvert.DeserializeObject<AppListQuery>(property.Value.ToString());
                }
                else
                {
                    appDesiredStates.Add(JsonToAppDesiredState(property.Name, property.Value));
                }
            }

            return new DesiredActions(appDesiredStates, appListQuery);
        }

        private static async Task<AppReportedState> UpdateAppFromStore(AppDesiredState desiredState)
        {
            Debug.WriteLine("Updating " + desiredState.packageFamilyName + " from store");
            return new AppReportedState(
                desiredState.packageFamilyId,
                desiredState.packageFamilyName,
                desiredState.version.ToString(),
                desiredState.startUp,
                "",
                new Error(ErrorCodes.E_NOTIMPL, "Updating app from the store has not been implemented yet."),
                JsonReport.Report);
        }

        private async Task<IDictionary<string, Message.AppInfo>> ListAppsAsync()
        {
            var request = new Message.ListAppsRequest();
            var result = await this._systemConfiguratorProxy.SendCommandAsync(request);
            return (result as Message.ListAppsResponse).Apps;
        }

        private async Task<Message.AppInstallResponse> InstallAppAsync(Message.AppInstallRequestData requestData)
        {
            Debug.WriteLine("Installing: " + requestData.PackageFamilyName);

            var request = new Message.AppInstallRequest(requestData);
            return await this._systemConfiguratorProxy.SendCommandAsync(request) as Message.AppInstallResponse;
        }

        private async Task<Message.AppUninstallResponse> UninstallAppAsync(Message.AppUninstallRequestData requestData)
        {
            Debug.WriteLine("Uninstalling: " + requestData.PackageFamilyName);

            var request = new Message.AppUninstallRequest(requestData);
            return await this._systemConfiguratorProxy.SendCommandAsync(request) as Message.AppUninstallResponse;
        }

        private async Task UninstallAppAsync(AppInfo appInfo, string packageFamilyId, string packageFamilyName)
        {
            Debug.WriteLine("Processing uninstall request...");

            AppReportedState reportedState = null;
            try
            {
                // ToDo: We need to handle store and system apps too.
                AppUninstallResponse response = await UninstallAppAsync(new AppUninstallRequestData(packageFamilyName, false /*non-store app*/));
                if (response.Status == ResponseStatus.Success)
                {
                    reportedState = new AppReportedState(packageFamilyId,
                                                         packageFamilyName,
                                                         VersionNotInstalled,
                                                         StartUpType.None,
                                                         null,   // no install date
                                                         null,   // no error
                                                         JsonReport.Report);
                    _stateToReport[packageFamilyId] = reportedState;
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
                                                     appInfo.StartUp,
                                                     appInfo.InstallDate,
                                                     e,
                                                     JsonReport.Report);
            }
            catch (Exception e)
            {
                reportedState = new AppReportedState(packageFamilyId,
                                                     packageFamilyName,
                                                     appInfo.Version,
                                                     appInfo.StartUp,
                                                     appInfo.InstallDate,
                                                     new Error(e.HResult, e.Message),
                                                     JsonReport.Report);
            }
            _stateToReport[packageFamilyId] = reportedState;
            throw new Exception("Failed to uninstall " + packageFamilyName);
        }

        private async Task InstallAppFromAzureAsync(AppInfo currentState, string connectionString, AppDesiredState desiredState)
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
                requestData.StartUp = desiredState.startUp;

                // Downloading dependencies...
                Debug.WriteLine(DateTime.Now.ToString("HH:mm:ss") + " Downloading...");
                if (!String.IsNullOrEmpty(desiredState.depsSources))
                {
                    string[] depsSources = desiredState.depsSources.Split(';');

                    for (int i = 0; i < depsSources.Length; ++i)
                    {
                        IoTDMClient.BlobInfo dependencyBlob = IoTDMClient.BlobInfo.BlobInfoFromSource(connectionString, depsSources[i]);
                        Debug.WriteLine(DateTime.Now.ToString("HH:mm:ss") + " Downloading " + dependencyBlob.BlobName);

                        var dependencyPath = await dependencyBlob.DownloadToTempAsync(this._systemConfiguratorProxy);
                        Debug.WriteLine(dependencyPath);
                        requestData.Dependencies.Add(dependencyPath);
                    }
                }

                // Downloading certificates...
                Debug.WriteLine(DateTime.Now.ToString("HH:mm:ss") + " Downloading appx certificate...");
                IoTDMClient.BlobInfo certificateBlob = IoTDMClient.BlobInfo.BlobInfoFromSource(connectionString, desiredState.certSource);
                requestData.CertFile = await certificateBlob.DownloadToTempAsync(this._systemConfiguratorProxy);
                requestData.CertStore = desiredState.certStore;

                // Downloading appx...
                Debug.WriteLine(DateTime.Now.ToString("HH:mm:ss") + " Downloading appx...");
                IoTDMClient.BlobInfo appxBlob = IoTDMClient.BlobInfo.BlobInfoFromSource(connectionString, desiredState.appxSource);
                requestData.AppxPath = await appxBlob.DownloadToTempAsync(this._systemConfiguratorProxy);

                // Installing appx...
                Debug.WriteLine(DateTime.Now.ToString("HH:mm:ss") + " Installing appx...");
                AppInstallResponse response = await InstallAppAsync(requestData);
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
                                                         response.data.startUp,
                                                         response.data.installDate,
                                                         e,
                                                         JsonReport.Report);
                    _stateToReport[desiredState.packageFamilyId] = reportedState;
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
                                                     currentState.StartUp,
                                                     currentState.InstallDate,   // install date
                                                     e,
                                                     JsonReport.Report);
            }
            catch (Exception e)
            {
                reportedState = new AppReportedState(desiredState.packageFamilyId,
                                                     desiredState.packageFamilyName,
                                                     currentState.Version,
                                                     currentState.StartUp,
                                                     currentState.InstallDate,   // install date
                                                     new Error(e.HResult, e.Message),
                                                     JsonReport.Report);
            }
            _stateToReport[desiredState.packageFamilyId] = reportedState;
            throw new Exception("Failed to install " + desiredState.packageFamilyName);
        }

        private async Task<StartUpType> GetAppStartup(string appId)
        {
            string foregroundApp = await GetStartupForegroundAppAsync();
            if (appId == foregroundApp)
            {
                return StartUpType.Foreground;
            }

            IList<string> backgroundApps = await ListStartupBackgroundAppsAsync();
            foreach(string s in backgroundApps)
            {
                Debug.WriteLine("Found background app: " + s);
            }
            var result = from backgroundApp in backgroundApps where backgroundApp == appId select backgroundApp;
            if (result.Count<string>() > 0)
            {
                return StartUpType.Background;
            }

            return StartUpType.None;
        }

        private async Task InstallAppAsync(AppInfo installedAppInfo, Version installedAppVersion, string connectionString, IDictionary<string, AppInfo> installedApps, AppDesiredState desiredState)
        {
            Debug.WriteLine("Processing install request for " + desiredState.packageFamilyId);

            if (installedAppInfo == null)
            {
                // It is a new application.
                Debug.WriteLine("    Can't find an installed version... Installing a fresh copy...");
                await InstallAppFromAzureAsync(installedAppInfo, connectionString, desiredState);       // ---> InstallAppFromAzureAsync
            }
            else
            {
                // A version of this application is installed.
                Debug.WriteLine("    Found an installed version...");

                if (desiredState.version == installedAppVersion)
                {
                    Debug.WriteLine("        Same version is installed...");
                    AppReportedState appReportedState = null;

                    if (desiredState.startUp == installedAppInfo.StartUp)
                    {
                        Debug.WriteLine("        App StartUp is the same: desired = " + desiredState.startUp);

                        appReportedState = new AppReportedState(
                            desiredState.packageFamilyId,
                            desiredState.packageFamilyName,
                            installedAppVersion.ToString(),
                            installedAppInfo.StartUp,
                            installedAppInfo.InstallDate,
                            null,                 // no error
                            JsonReport.Report);
                    }
                    else
                    {
                        Debug.WriteLine("        App StartUp is different: desired = " + desiredState.startUp.ToString() + ", current = " + installedAppInfo.StartUp.ToString());

                        switch (desiredState.startUp)
                        {
                            case StartUpType.None:
                                {
                                    Debug.WriteLine("            Removing app from background apps.");
                                    StartupAppInfo startupAppInfo = new StartupAppInfo(desiredState.packageFamilyName, true /*background*/);
                                    await RemoveStartupAppAsync(startupAppInfo);
                                }
                                break;
                            case StartUpType.Foreground:
                                {
                                    Debug.WriteLine("            Setting app to be the foreground app.");
                                    StartupAppInfo startupAppInfo = new StartupAppInfo(desiredState.packageFamilyName, false /*background*/);
                                    await AddStartupAppAsync(startupAppInfo);
                                }
                                break;
                            case StartUpType.Background:
                                {
                                    Debug.WriteLine("            Adding app to the background apps.");
                                    StartupAppInfo startupAppInfo = new StartupAppInfo(desiredState.packageFamilyName, true /*background*/);
                                    await AddStartupAppAsync(startupAppInfo);
                                }
                                break;
                        }

                        StartUpType appStartUp = await GetAppStartup(desiredState.packageFamilyName);
                        Debug.WriteLine("            Querying returned app startup: " + appStartUp.ToString());

                        appReportedState = new AppReportedState(
                            desiredState.packageFamilyId,
                            desiredState.packageFamilyName,
                            installedAppVersion.ToString(),
                            appStartUp,
                            installedAppInfo.InstallDate,
                            null,                 // no error
                            JsonReport.Report);
                    }

                    _stateToReport[desiredState.packageFamilyId] = appReportedState;
                }
                else if (desiredState.version > installedAppVersion)
                {
                    Debug.WriteLine("        Older version is installed...");

                    if (!String.IsNullOrEmpty(desiredState.appxSource))
                    {
                        await InstallAppFromAzureAsync(installedAppInfo, connectionString, desiredState);
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
                            installedAppInfo.StartUp,
                            installedAppInfo.InstallDate,
                            new Error(ErrorCodes.INVALID_DESIRED_APPX_SRC, "Cannot install appx without a source."),
                            JsonReport.Report);

                        _stateToReport[desiredState.packageFamilyId] = appReportedState;

                        throw new Exception("Failed to roll back application version.");
                    }
                    else
                    {
                        // Note that UninstallAppAsync will throw if it fails - and correctly avoid launching the install...
                        await UninstallAppAsync(installedAppInfo, desiredState.packageFamilyId, desiredState.packageFamilyId);
                        await InstallAppFromAzureAsync(installedAppInfo, connectionString, desiredState);
                    }
                }
            }
        }

        private async Task QueryAppAsync(AppInfo installedAppInfo, AppDesiredState desiredState)
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
                    StartUpType.None,
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
                    installedAppInfo.StartUp,
                    installedAppInfo.InstallDate,
                    null,
                    JsonReport.Report);
            }
            _stateToReport[desiredState.packageFamilyId] = appReportedState;
        }

        private async Task ApplyAppDesiredState(string connectionString, IDictionary<string, AppInfo> installedApps, AppDesiredState desiredState)
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
                        await InstallAppAsync(installedAppInfo, installedAppVersion, connectionString, installedApps, desiredState);
                    }
                    break;
                case AppDesiredAction.Uninstall:
                    {
                        await UninstallAppAsync(installedAppInfo, desiredState.packageFamilyId, desiredState.packageFamilyId);
                    }
                    break;
                case AppDesiredAction.Query:
                    {
                        await QueryAppAsync(installedAppInfo, desiredState);
                    }
                    break;
                case AppDesiredAction.Unreport:
                    {
                        Debug.WriteLine("Processing unreport request...");

                        AppReportedState appReportedState = new AppReportedState(
                            desiredState.packageFamilyId,
                            desiredState.packageFamilyName,
                            null,   // no version
                            StartUpType.None,
                            null,   // no install date
                            null,   // no error
                            JsonReport.Unreport);

                        _stateToReport[desiredState.packageFamilyId] = appReportedState;
                    }
                    break;
                case AppDesiredAction.Undefined:
                    {
                        Debug.WriteLine("Processing unknown request...");

                        AppReportedState appReportedState = new AppReportedState(
                            desiredState.packageFamilyId,
                            desiredState.packageFamilyName,
                            null,   // no version
                            StartUpType.None,
                            null,   // no install date
                            new Error(ErrorCodes.INVALID_DESIRED_APPX_OPERATION, "Invalid application operation."),
                            JsonReport.Report);

                        _stateToReport[desiredState.packageFamilyId] = appReportedState;
                    }
                    break;
            }
        }

        private async Task ApplyDesiredAppsConfiguration(JToken jAppsToken)
        {
            if (!(jAppsToken is JObject))
            {
                return;
            }

            try
            {
                JObject appsNode = (JObject)jAppsToken;

                // Reset the nodes to report...
                _stateToReport = new Dictionary<string, AppReportedState>();

                IDictionary<string, AppInfo> installedApps = await ListAppsAsync();
                DumpInstalledApps(installedApps);

                DesiredActions desiredActions = JsonToDesiredActions(appsNode);

                ReorderAndValidate(desiredActions.appDesiredStates, installedApps);

                foreach (AppDesiredState appDesiredState in desiredActions.appDesiredStates)
                {
                    try
                    {
                        await ApplyAppDesiredState(this._connectionString, installedApps, appDesiredState);
                    }
                    catch (Exception)
                    {
                        // Catch everything here so that we may continue processing other appDesiredStates.
                        // No reporting to the IoT Hub is needed here because it has already taken place.
                    }
                }

                // Process the "?"
                if (desiredActions.appListQuery.nonStore || desiredActions.appListQuery.store)
                {
                    foreach (var pair in installedApps)
                    {
                        AppReportedState dummyAppReportedState;
                        if (_stateToReport.TryGetValue(pair.Value.PackageFamilyName, out dummyAppReportedState))
                        {
                            // Already queued to be reported, no need to consider adding it to _stateToReport.
                            continue;
                        }
                        if ((pair.Value.AppSource == "AppStore" && desiredActions.appListQuery.store) ||
                             (pair.Value.AppSource == "NonStore" && desiredActions.appListQuery.nonStore))
                        {
                            AppReportedState appReportedState = new AppReportedState(
                                pair.Value.PackageFamilyName.Replace('.', '_'),
                                pair.Value.PackageFamilyName,
                                pair.Value.Version,
                                pair.Value.StartUp,
                                pair.Value.InstallDate,
                                null,
                                JsonReport.Report);

                            _stateToReport[pair.Value.PackageFamilyName] = appReportedState;
                        }
                    }
                }

                // Reset the apps reported node...
                await NullifyReported();

                // Report all collected values...
                foreach (var pair in _stateToReport)
                {
                    await ReportAppStatus(pair.Value);
                }
            }
            catch(Exception e)
            {
                await NullifyReported();
                await ReportGeneralError(e.HResult, e.Message);
            }
        }

        private async Task<string> GetStartupForegroundAppAsync()
        {
            var request = new Message.GetStartupForegroundAppRequest();
            var result = await this._systemConfiguratorProxy.SendCommandAsync(request);
            return (result as Message.GetStartupForegroundAppResponse).StartupForegroundApp;
        }

        private async Task<IList<string>> ListStartupBackgroundAppsAsync()
        {
            var request = new Message.ListStartupBackgroundAppsRequest();
            var result = await this._systemConfiguratorProxy.SendCommandAsync(request);
            return (result as Message.ListStartupBackgroundAppsResponse).StartupBackgroundApps;
        }

        private async Task AddStartupAppAsync(Message.StartupAppInfo startupAppInfo)
        {
            var request = new Message.AddStartupAppRequest(startupAppInfo);
            await this._systemConfiguratorProxy.SendCommandAsync(request);
        }

        private async Task RemoveStartupAppAsync(Message.StartupAppInfo startupAppInfo)
        {
            var request = new Message.RemoveStartupAppRequest(startupAppInfo);
            await this._systemConfiguratorProxy.SendCommandAsync(request);
        }

        private async Task AppLifecycleAsync(Message.AppLifecycleInfo appInfo)
        {
            var request = new Message.AppLifecycleRequest(appInfo);
            await this._systemConfiguratorProxy.SendCommandAsync(request);
        }

        private void UpdateCache(JToken desiredValue)
        {
            JToken cachedToken = _desiredCache.SelectToken(JsonSectionName);
            if (cachedToken != null)
            {
                if (cachedToken is JObject)
                {
                    JObject cachedObject = (JObject)cachedToken;
                    cachedObject.Merge(desiredValue);
                }
            }
            else
            {
                _desiredCache[JsonSectionName] = desiredValue;
            }
        }

        // IClientPropertyHandler
        public void OnDesiredPropertyChange(JToken desiredValue)
        {
            UpdateCache(desiredValue);

            // Need to revisit all the desired nodes (not only the changed ones) so that we can re-construct the correct reported list.
            ApplyDesiredAppsConfiguration(_desiredCache[JsonSectionName]);
        }

        // IClientPropertyHandler
        public async Task<JObject> GetReportedPropertyAsync()
        {
            // ToDo: we need to use the cached status to know what to report back.
            return (JObject)JsonConvert.DeserializeObject("{ \"state\" : \"not implemented\" }");
        }

        private string _connectionString;
        private ISystemConfiguratorProxy _systemConfiguratorProxy;
        private IClientHandlerCallBack _callback;
        private JObject _desiredCache;
        private Dictionary<string, AppReportedState> _stateToReport;
        
    }
}
