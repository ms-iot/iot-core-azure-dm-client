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

using Microsoft.Devices.Management.DMDataContract;
using Microsoft.Devices.Management.Message;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Foundation.Diagnostics;

namespace Microsoft.Devices.Management
{
    class AppUtils
    {
        public static Message.StartUpType StartUpTypeToMessage(AppxStartUpType value)
        {
            switch (value)
            {
                case AppxStartUpType.Foreground:
                    return Message.StartUpType.Foreground;
                case AppxStartUpType.Background:
                    return Message.StartUpType.Background;
            }
            return Message.StartUpType.None;
        }

        public static AppxStartUpType StartUpTypeFromMessage(Message.StartUpType value)
        {
            switch (value)
            {
                case Message.StartUpType.Foreground:
                    return AppxStartUpType.Foreground;
                case Message.StartUpType.Background:
                    return AppxStartUpType.Background;
            }
            return AppxStartUpType.None;
        }

        public static void DumpAppDesiredState(AppxManagementDataContract.AppDesiredState appDesiredState)
        {
                Logger.Log("- Desired App State ------------------------------------------------", LoggingLevel.Verbose);
                Logger.Log("    packageFamilyId     = " + appDesiredState.packageFamilyId, LoggingLevel.Verbose);
                Logger.Log("    packageFamilyName   = " + appDesiredState.packageFamilyName, LoggingLevel.Verbose);
                Logger.Log("    action              = " + appDesiredState.action.ToString(), LoggingLevel.Verbose);
                Logger.Log("    version             = " + appDesiredState.version?.ToString(), LoggingLevel.Verbose);
                Logger.Log("    startUp             = " + appDesiredState.startUp.ToString(), LoggingLevel.Verbose);
                Logger.Log("    appxSource          = " + appDesiredState.appxSource, LoggingLevel.Verbose);
                Logger.Log("    depsSources         = " + appDesiredState.depsSources, LoggingLevel.Verbose);
                Logger.Log("    certSource          = " + appDesiredState.certSource, LoggingLevel.Verbose);
                Logger.Log("    certStore           = " + appDesiredState.certStore, LoggingLevel.Verbose);
                if (appDesiredState.errorCode != 0)
                {
                    Logger.Log("    errorCode           = " + appDesiredState.errorCode, LoggingLevel.Verbose);
                    Logger.Log("    errorMessage        = " + appDesiredState.errorMessage, LoggingLevel.Verbose);
                }
                Logger.Log("    ----------------------------------------------------------------", LoggingLevel.Verbose);
        }

        public static void DumpDesiredProperties(AppxManagementDataContract.DesiredProperties desiredProperties)
        {
            Logger.Log("- Desired Properties ------------------------------------------------", LoggingLevel.Verbose);
            Logger.Log("appListQuery.store     = " + desiredProperties.appListQuery.store, LoggingLevel.Verbose);
            Logger.Log("appListQuery.nonStore  = " + desiredProperties.appListQuery.nonStore, LoggingLevel.Verbose);
            foreach (AppxManagementDataContract.AppDesiredState appDesiredState in desiredProperties.appDesiredStates)
            {
                DumpAppDesiredState(appDesiredState);
            }
        }
    }

    class AppxManagement : IClientPropertyHandler, IClientPropertyDependencyHandler
    {
        // IClientPropertyHandler
        public string PropertySectionName
        {
            get
            {
                return AppxManagementDataContract.SectionName;
            }
        }

        string[] JsonSectionDependencyNames = { ExternalStorageDataContract.SectionName };

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
                this._connectionString = (string)value.Property(ExternalStorageDataContract.JsonConnectionString).Value;
            }
        }

        private static void DumpInstalledApps(IDictionary<string, AppInfo> data)
        {
            foreach (KeyValuePair<string, AppInfo> p in data)
            {
                Logger.Log("- Installed App ---------------------------------------------", LoggingLevel.Verbose);
                Logger.Log("Key               = " + p.Key, LoggingLevel.Verbose);
                Logger.Log("Name              = " + p.Value.Name, LoggingLevel.Verbose);
                Logger.Log("PackageFamilyName = " + p.Value.PackageFamilyName, LoggingLevel.Verbose);
                Logger.Log("AppSource         = " + p.Value.AppSource, LoggingLevel.Verbose);
                Logger.Log("InstallDate       = " + p.Value.InstallDate, LoggingLevel.Verbose);
                Logger.Log("InstallLocation   = " + p.Value.InstallLocation, LoggingLevel.Verbose);
                Logger.Log("Architecture      = " + p.Value.Architecture, LoggingLevel.Verbose);
                Logger.Log("Publisher         = " + p.Value.Publisher, LoggingLevel.Verbose);
                Logger.Log("Version           = " + p.Value.Version, LoggingLevel.Verbose);
                Logger.Log("Users             = " + p.Value.Users, LoggingLevel.Verbose);
                Logger.Log("StartUp           = " + p.Value.StartUp, LoggingLevel.Verbose);
                Logger.Log("-------------------------------------------------------------", LoggingLevel.Verbose);
            }
        }

        private async Task ReportAppStatus(AppxManagementDataContract.AppReportedState appReportedState)
        {
            JObject reportedStateObject = appReportedState.ToJsonObject();
            Logger.Log("Report:\n" + reportedStateObject.ToString(), LoggingLevel.Verbose);
            await this._callback.ReportPropertiesAsync(PropertySectionName, reportedStateObject);
        }

        private async Task NullifyReported()
        {
            Logger.Log("NullifyReported\n", LoggingLevel.Verbose);
            await this._callback.ReportPropertiesAsync(PropertySectionName, new JValue(CommonDataContract.JsonRefreshing));
        }

        private void ReorderAndValidate(List<AppxManagementDataContract.AppDesiredState> appDesiredStates, IDictionary<string, AppInfo> installedApps)
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

            AppxManagementDataContract.AppDesiredState desiredForegroundApp = null;
            var foregroundApps = from state in appDesiredStates
                         where state.startUp == AppxStartUpType.Foreground
                         select state;

            foreach (AppxManagementDataContract.AppDesiredState state in foregroundApps)
            {
                Logger.Log("Found foreground app: " + state.packageFamilyName, LoggingLevel.Verbose);
                desiredForegroundApp = state;
            }

            if (foregroundApps.Count<AppxManagementDataContract.AppDesiredState>() > 1)
            {
                StringBuilder sb = new StringBuilder();
                foreach(AppxManagementDataContract.AppDesiredState s in foregroundApps)
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
            if (desiredForegroundApp != null && 
                desiredForegroundApp.startUp == AppxStartUpType.Foreground && 
                desiredForegroundApp.action == AppDesiredAction.Uninstall)
            {
                // This means that no other application has been set to replace the one about to be uninstalled.
                throw new Error(ErrorCodes.INVALID_DESIRED_CONFLICT_UNINSTALL_FOREGROUND_APP, "Cannot configure an application to the foreground application and uninstall it. Package Family Name = " + desiredForegroundApp.packageFamilyName);
            }

            // Make sure that the application being uninstalled is not the foreground app as marked by the system (i.e. installedApps).
            // This is to catch the case where the desired state does not say anything about foreground apps - but the system does.
            // So, we want to make sure the desired state does not conflict with the system state.
            var uninstallForegroundApps = from desiredState in appDesiredStates
                                          join installedState in installedApps on desiredState.packageFamilyName equals installedState.Value.PackageFamilyName
                                          where desiredState.action == AppDesiredAction.Uninstall && installedState.Value.StartUp == Message.StartUpType.Foreground
                                          select desiredState;

            if (uninstallForegroundApps.Count<AppxManagementDataContract.AppDesiredState>() > 0)
            {
                // There should be at most one application matching since the foreground setting on the system allows only one foreground application.
                string foregroundAppToUninstall = uninstallForegroundApps.First<AppxManagementDataContract.AppDesiredState>().packageFamilyName;

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

            Logger.Log("Ordered:", LoggingLevel.Verbose);
            foreach (AppxManagementDataContract.AppDesiredState state in appDesiredStates)
            {
                Logger.Log("App: " + state.packageFamilyName + " " + state.startUp.ToString(), LoggingLevel.Verbose);
            }
        }

        private async Task<IDictionary<string, Message.AppInfo>> ListAppsAsync()
        {
            var request = new Message.ListAppsRequest();
            var result = await this._systemConfiguratorProxy.SendCommandAsync(request);
            return (result as Message.ListAppsResponse).Apps;
        }

        private async Task<Message.AppInstallResponse> InstallAppAsync(Message.AppInstallRequestData requestData)
        {
            Logger.Log("Installing: " + requestData.PackageFamilyName, LoggingLevel.Verbose);

            var request = new Message.AppInstallRequest(requestData);
            return await this._systemConfiguratorProxy.SendCommandAsync(request) as Message.AppInstallResponse;
        }

        private async Task<Message.AppUninstallResponse> UninstallAppAsync(Message.AppUninstallRequestData requestData)
        {
            Logger.Log("Uninstalling: " + requestData.PackageFamilyName, LoggingLevel.Verbose);

            var request = new Message.AppUninstallRequest(requestData);
            return await this._systemConfiguratorProxy.SendCommandAsync(request) as Message.AppUninstallResponse;
        }

        private async Task UninstallAppAsync(AppInfo appInfo, string packageFamilyId, string packageFamilyName)
        {
            Logger.Log("Processing uninstall request...", LoggingLevel.Verbose);

            AppxManagementDataContract.AppReportedState reportedState = null;
            try
            {
                Windows.ApplicationModel.PackageId thisPackage = Windows.ApplicationModel.Package.Current.Id;
                Debug.WriteLine("FamilyName = " + thisPackage.FamilyName);
                Debug.WriteLine("Name       = " + thisPackage.Name);

                if (packageFamilyName == thisPackage.FamilyName)
                {
                    throw new Error(ErrorCodes.CANNOT_UNINSTALL_DM_APPLICATION, "Cannot uninstall the DM application: " + thisPackage.FamilyName);
                }

                // ToDo: We need to handle store and system apps too.
                AppUninstallResponse response = await UninstallAppAsync(new AppUninstallRequestData(packageFamilyName, false /*non-store app*/));

                reportedState = new AppxManagementDataContract.AppReportedState(packageFamilyId,
                                                        packageFamilyName,
                                                        AppxManagementDataContract.VersionNotInstalled,
                                                        AppxStartUpType.None,
                                                        null,   // no install date
                                                        0,   // no error
                                                        null,
                                                        JsonReport.Report);
                _stateToReport[packageFamilyId] = reportedState;
                return;
            }
            catch(Error e)
            {
                reportedState = new AppxManagementDataContract.AppReportedState(packageFamilyId,
                                                     packageFamilyName,
                                                     appInfo.Version,
                                                     AppUtils.StartUpTypeFromMessage(appInfo.StartUp),
                                                     appInfo.InstallDate,
                                                     e.HResult,
                                                     e.Message,
                                                     JsonReport.Report);
            }
            catch (Exception e)
            {
                reportedState = new AppxManagementDataContract.AppReportedState(packageFamilyId,
                                                     packageFamilyName,
                                                     appInfo.Version,
                                                     AppUtils.StartUpTypeFromMessage(appInfo.StartUp),
                                                     appInfo.InstallDate,
                                                     e.HResult,
                                                     e.Message,
                                                     JsonReport.Report);
            }
            _stateToReport[packageFamilyId] = reportedState;
            throw new Exception("Failed to uninstall " + packageFamilyName);
        }

        private async Task InstallAppFromAzureAsync(AppInfo currentState, string connectionString, AppxManagementDataContract.AppDesiredState desiredState, bool selfUpdate)
        {
            // Is this a fresh installation?
            if (currentState == null)
            {
                currentState = new AppInfo();
                currentState.PackageFamilyName = desiredState.packageFamilyName;
                currentState.Version = AppxManagementDataContract.VersionNotInstalled;
                currentState.InstallDate = "";
            }

            AppxManagementDataContract.AppReportedState reportedState = null;
            try
            {
                Logger.Log("-> installing " + desiredState.packageFamilyName + " from " + desiredState.appxSource, LoggingLevel.Verbose);
                if (String.IsNullOrEmpty(desiredState.appxSource))
                {
                    throw new Error(ErrorCodes.INVALID_DESIRED_APPX_SRC, "Cannot install appx without a source.");
                }

                var requestData = new AppInstallRequestData();
                requestData.PackageFamilyName = desiredState.packageFamilyName;
                requestData.StartUp = AppUtils.StartUpTypeToMessage(desiredState.startUp);

                // Downloading dependencies...
                Logger.Log(DateTime.Now.ToString("HH:mm:ss") + " Downloading...", LoggingLevel.Verbose);
                if (!String.IsNullOrEmpty(desiredState.depsSources))
                {
                    string[] depsSources = desiredState.depsSources.Split(';');

                    for (int i = 0; i < depsSources.Length; ++i)
                    {
                        IoTDMClient.BlobInfo dependencyBlob = IoTDMClient.BlobInfo.BlobInfoFromSource(connectionString, depsSources[i]);
                        Logger.Log(DateTime.Now.ToString("HH:mm:ss") + " Downloading " + dependencyBlob.BlobName, LoggingLevel.Verbose);

                        var dependencyPath = await dependencyBlob.DownloadToTempAsync(this._systemConfiguratorProxy);
                        Logger.Log(dependencyPath, LoggingLevel.Verbose);
                        requestData.Dependencies.Add(dependencyPath);
                    }
                }

                // Downloading certificates...
                Logger.Log(DateTime.Now.ToString("HH:mm:ss") + " Downloading appx certificate...", LoggingLevel.Verbose);
                IoTDMClient.BlobInfo certificateBlob = IoTDMClient.BlobInfo.BlobInfoFromSource(connectionString, desiredState.certSource);
                requestData.CertFile = await certificateBlob.DownloadToTempAsync(this._systemConfiguratorProxy);
                requestData.CertStore = desiredState.certStore;

                // Downloading appx...
                Logger.Log(DateTime.Now.ToString("HH:mm:ss") + " Downloading appx...", LoggingLevel.Verbose);
                IoTDMClient.BlobInfo appxBlob = IoTDMClient.BlobInfo.BlobInfoFromSource(connectionString, desiredState.appxSource);
                requestData.AppxPath = await appxBlob.DownloadToTempAsync(this._systemConfiguratorProxy);
                requestData.IsDMSelfUpdate = selfUpdate;

                // Installing appx...
                Logger.Log(DateTime.Now.ToString("HH:mm:ss") + " Installing appx...", LoggingLevel.Verbose);
                AppInstallResponse response = await InstallAppAsync(requestData);
                if (response.data.pending)
                {
                    Logger.Log(DateTime.Now.ToString("HH:mm:ss") + " Installing appx is pending...", LoggingLevel.Verbose);
                    Debug.Assert(selfUpdate);

                    reportedState = new AppxManagementDataContract.AppReportedState(desiredState.packageFamilyId,
                                                        desiredState.packageFamilyName,
                                                        CommonDataContract.JsonPending,
                                                        AppUtils.StartUpTypeFromMessage(response.data.startUp),
                                                        response.data.installDate,
                                                        0,
                                                        null,
                                                        JsonReport.Report);
                    _stateToReport[desiredState.packageFamilyId] = reportedState;
                    return;
                }
                else
                {
                    Logger.Log(DateTime.Now.ToString("HH:mm:ss") + " Done installing appx...", LoggingLevel.Verbose);

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

                    reportedState = new AppxManagementDataContract.AppReportedState(desiredState.packageFamilyId,
                                                        desiredState.packageFamilyName,
                                                        response.data.version,
                                                        AppUtils.StartUpTypeFromMessage(response.data.startUp),
                                                        response.data.installDate,
                                                        (e != null? e.HResult : 0),
                                                        (e != null ? e.Message : null),
                                                        JsonReport.Report);
                    _stateToReport[desiredState.packageFamilyId] = reportedState;
                    return;
                }
            }
            catch (Error e)
            {
                reportedState = new AppxManagementDataContract.AppReportedState(desiredState.packageFamilyId,
                                                     desiredState.packageFamilyName,
                                                     currentState.Version,
                                                     AppUtils.StartUpTypeFromMessage(currentState.StartUp),
                                                     currentState.InstallDate,   // install date
                                                     e.HResult,
                                                     e.Message,
                                                     JsonReport.Report);
            }
            catch (Exception e)
            {
                reportedState = new AppxManagementDataContract.AppReportedState(desiredState.packageFamilyId,
                                                     desiredState.packageFamilyName,
                                                     currentState.Version,
                                                     AppUtils.StartUpTypeFromMessage(currentState.StartUp),
                                                     currentState.InstallDate,   // install date
                                                     e.HResult,
                                                     e.Message,
                                                     JsonReport.Report);
            }
            _stateToReport[desiredState.packageFamilyId] = reportedState;
            throw new Exception("Failed to install " + desiredState.packageFamilyName);
        }

        private async Task<AppxStartUpType> GetAppStartup(string appId)
        {
            string foregroundApp = await GetStartupForegroundAppAsync();
            if (appId == foregroundApp)
            {
                return AppxStartUpType.Foreground;
            }

            IList<string> backgroundApps = await ListStartupBackgroundAppsAsync();
            foreach(string s in backgroundApps)
            {
                Logger.Log("Found background app: " + s, LoggingLevel.Verbose);
            }
            var result = from backgroundApp in backgroundApps where backgroundApp == appId select backgroundApp;
            if (result.Count<string>() > 0)
            {
                return AppxStartUpType.Background;
            }

            return AppxStartUpType.None;
        }

        private async Task<CommandStatus> InstallAppAsync(
            AppInfo installedAppInfo,
            Version installedAppVersion,
            string connectionString,
            IDictionary<string, AppInfo> installedApps,
            AppxManagementDataContract.AppDesiredState desiredState)
        {
            Logger.Log("Processing install request for " + desiredState.packageFamilyId, LoggingLevel.Verbose);

            if (installedAppInfo == null)
            {
                // It is a new application.
                Logger.Log("    Can't find an installed version... Installing a fresh copy...", LoggingLevel.Verbose);
                await InstallAppFromAzureAsync(installedAppInfo, connectionString, desiredState, false /*not self update*/);       // ---> InstallAppFromAzureAsync
            }
            else
            {
                Windows.ApplicationModel.PackageId thisPackage = Windows.ApplicationModel.Package.Current.Id;
                Debug.WriteLine("FamilyName = " + thisPackage.FamilyName);
                Debug.WriteLine("Name       = " + thisPackage.Name);

                bool isSelf = desiredState.packageFamilyName == thisPackage.FamilyName;

                // A version of this application is installed.
                Logger.Log("    Found an installed version...", LoggingLevel.Verbose);

                if (desiredState.version == installedAppVersion)
                {
                    Logger.Log("        Same version is installed...", LoggingLevel.Verbose);
                    AppxManagementDataContract.AppReportedState appReportedState = null;

                    if (AppUtils.StartUpTypeToMessage(desiredState.startUp) == installedAppInfo.StartUp)
                    {
                        Logger.Log("        App StartUp is the same: desired = " + desiredState.startUp, LoggingLevel.Verbose);

                        appReportedState = new AppxManagementDataContract.AppReportedState(
                            desiredState.packageFamilyId,
                            desiredState.packageFamilyName,
                            installedAppVersion.ToString(),
                            AppUtils.StartUpTypeFromMessage(installedAppInfo.StartUp),
                            installedAppInfo.InstallDate,
                            0,
                            null,                 // no error
                            JsonReport.Report);
                    }
                    else
                    {
                        Logger.Log("        App StartUp is different: desired = " + desiredState.startUp.ToString() + ", current = " + installedAppInfo.StartUp.ToString(), LoggingLevel.Verbose);

                        switch (desiredState.startUp)
                        {
                            case AppxStartUpType.None:
                                {
                                    Logger.Log("            Removing app from background apps.", LoggingLevel.Verbose);
                                    StartupAppInfo startupAppInfo = new StartupAppInfo(desiredState.packageFamilyName, true /*background*/);
                                    await RemoveStartupAppAsync(startupAppInfo);
                                }
                                break;
                            case AppxStartUpType.Foreground:
                                {
                                    Logger.Log("            Setting app to be the foreground app.", LoggingLevel.Verbose);
                                    StartupAppInfo startupAppInfo = new StartupAppInfo(desiredState.packageFamilyName, false /*background*/);
                                    await AddStartupAppAsync(startupAppInfo);
                                }
                                break;
                            case AppxStartUpType.Background:
                                {
                                    Logger.Log("            Adding app to the background apps.", LoggingLevel.Verbose);
                                    StartupAppInfo startupAppInfo = new StartupAppInfo(desiredState.packageFamilyName, true /*background*/);
                                    await AddStartupAppAsync(startupAppInfo);
                                }
                                break;
                        }

                        AppxStartUpType appStartUp = await GetAppStartup(desiredState.packageFamilyName);
                        Logger.Log("            Querying returned app startup: " + appStartUp.ToString(), LoggingLevel.Verbose);

                        appReportedState = new AppxManagementDataContract.AppReportedState(
                            desiredState.packageFamilyId,
                            desiredState.packageFamilyName,
                            installedAppVersion.ToString(),
                            appStartUp,
                            installedAppInfo.InstallDate,
                            0,
                            null,                 // no error
                            JsonReport.Report);
                    }

                    _stateToReport[desiredState.packageFamilyId] = appReportedState;
                }
                else if (desiredState.version > installedAppVersion)
                {
                    Logger.Log("        Older version is installed...", LoggingLevel.Verbose);

                    if (!String.IsNullOrEmpty(desiredState.appxSource))
                    {
                        // Trigger the update...
                        await InstallAppFromAzureAsync(installedAppInfo, connectionString, desiredState, isSelf);

                        if (isSelf)
                        {
                            // If isSelf == true, it means that SystemConfigurator will force this application to exit very soon.
                            // Let's stop processing any further desired properties.
                            return CommandStatus.PendingDMAppRestart;
                        }
                    }
                    else
                    {
                        // Note that store updates are not control through DM desired properties.
                        // Instead, they are triggered by the system scan for all store applications.
                        throw new Error(ErrorCodes.INVALID_DESIRED_APPX_SRC, "Appx package is required to update " + desiredState.packageFamilyName);
                    }
                }
                else
                {
                    // desiredState.version < installedAppVersion
                    Logger.Log("       Newer version is installed...rolling back.", LoggingLevel.Verbose);

                    if (String.IsNullOrEmpty(desiredState.appxSource))
                    {
                        AppxManagementDataContract.AppReportedState appReportedState = new AppxManagementDataContract.AppReportedState(
                            desiredState.packageFamilyId,
                            desiredState.packageFamilyName,
                            installedAppVersion.ToString(),
                            AppUtils.StartUpTypeFromMessage(installedAppInfo.StartUp),
                            installedAppInfo.InstallDate,
                            ErrorCodes.INVALID_DESIRED_APPX_SRC,
                            "Cannot install appx without a source.",
                            JsonReport.Report);

                        _stateToReport[desiredState.packageFamilyId] = appReportedState;

                        throw new Exception("Failed to roll back application version.");
                    }

                    if (isSelf)
                    {
                        // The reverting is implemented as an 'uninstall' followed by an 'install'.
                        // The package to be installed is downloaded in the 'install' step.
                        // So, if we were to revert self to an older version, we would have to:
                        //  - (1) download the old version first, 
                        //  - (2) send an atomic uninstall-install request to SystemConfigurator to
                        //        uninstall and follow with an install and re-launch.
                        //  The above two steps are not implemented yet.
                        throw new Error(ErrorCodes.CANNOT_REVERT_DM_APPLICATION, "Reverting to an older version of the device management application (" + desiredState.packageFamilyName + ") is not supported.");
                    }
                    // Note that UninstallAppAsync will throw if it fails - and correctly avoid launching the install...
                    await UninstallAppAsync(installedAppInfo, desiredState.packageFamilyId, desiredState.packageFamilyId);
                    await InstallAppFromAzureAsync(installedAppInfo, connectionString, desiredState, isSelf);
                }
            }

            return CommandStatus.Committed;
        }

        private static void ReportQueriedApp(
            AppInfo installedAppInfo,
            AppxManagementDataContract.AppDesiredState desiredState,
            Dictionary<string, AppxManagementDataContract.AppReportedState> stateToReport)
        {
            Logger.Log("Processing query request...", LoggingLevel.Verbose);

            AppxManagementDataContract.AppReportedState appReportedState;
            if (installedAppInfo == null)
            {
                Logger.Log("    Couldn't find an installed version...", LoggingLevel.Verbose);

                appReportedState = new AppxManagementDataContract.AppReportedState(
                    desiredState.packageFamilyId,
                    desiredState.packageFamilyName,
                    AppxManagementDataContract.VersionNotInstalled,
                    AppUtils.StartUpTypeFromMessage(StartUpType.None),
                    null,
                    0,
                    null,
                    JsonReport.Report);
            }
            else
            {
                Logger.Log("    Found an installed version...", LoggingLevel.Verbose);

                appReportedState = new AppxManagementDataContract.AppReportedState(
                    desiredState.packageFamilyId,
                    desiredState.packageFamilyName,
                    installedAppInfo.Version,
                    AppUtils.StartUpTypeFromMessage(installedAppInfo.StartUp),
                    installedAppInfo.InstallDate,
                    0,
                    null,
                    JsonReport.Report);
            }
            stateToReport[desiredState.packageFamilyId] = appReportedState;
        }

        private static AppInfo FindInstalledAppInfo(IDictionary<string, AppInfo> installedApps, string packageFamilyName)
        {
            AppInfo installedAppInfo = null;

            foreach (KeyValuePair<string, AppInfo> pair in installedApps)
            {
                AppInfo appInfo = pair.Value;
                if (appInfo.PackageFamilyName == packageFamilyName)
                {
                    installedAppInfo = appInfo;
                    break;
                }
            }

            return installedAppInfo;
        }

        private async Task<CommandStatus> ApplyAppDesiredState(string connectionString, IDictionary<string, AppInfo> installedApps, AppxManagementDataContract.AppDesiredState desiredState)
        {
            CommandStatus commandStatus = CommandStatus.NotStarted;
            Version installedAppVersion = null;
            AppInfo installedAppInfo = FindInstalledAppInfo(installedApps, desiredState.packageFamilyName);
            if (installedAppInfo != null)
            {
                installedAppVersion = new Version(installedAppInfo.Version);
            }

            switch (desiredState.action)
            {
                case AppDesiredAction.Install:
                    {
                        commandStatus = await InstallAppAsync(installedAppInfo, installedAppVersion, connectionString, installedApps, desiredState);
                    }
                    break;
                case AppDesiredAction.Uninstall:
                    {
                        await UninstallAppAsync(installedAppInfo, desiredState.packageFamilyId, desiredState.packageFamilyId);
                    }
                    break;
                case AppDesiredAction.Query:
                    {
                        ReportQueriedApp(installedAppInfo, desiredState, _stateToReport);
                    }
                    break;
                case AppDesiredAction.Unreport:
                    {
                        Logger.Log("Processing unreport request...", LoggingLevel.Verbose);

                        AppxManagementDataContract.AppReportedState appReportedState = new AppxManagementDataContract.AppReportedState(
                            desiredState.packageFamilyId,
                            desiredState.packageFamilyName,
                            null,   // no version
                            AppUtils.StartUpTypeFromMessage(StartUpType.None),
                            null,   // no install date
                            0,
                            null,   // no error
                            JsonReport.Unreport);

                        _stateToReport[desiredState.packageFamilyId] = appReportedState;
                    }
                    break;
                case AppDesiredAction.Undefined:
                    {
                        Logger.Log("Processing unknown request...", LoggingLevel.Verbose);

                        AppxManagementDataContract.AppReportedState appReportedState = new AppxManagementDataContract.AppReportedState(
                            desiredState.packageFamilyId,
                            desiredState.packageFamilyName,
                            null,   // no version
                            AppUtils.StartUpTypeFromMessage(StartUpType.None),
                            null,   // no install date
                            ErrorCodes.INVALID_DESIRED_APPX_OPERATION,
                            "Invalid application operation.",
                            JsonReport.Report);

                        _stateToReport[desiredState.packageFamilyId] = appReportedState;
                    }
                    break;
            }

            return commandStatus;
        }

        private async Task<CommandStatus> ApplyDesiredPropertiesAsync(JToken jAppsToken)
        {
            CommandStatus commandStatus = CommandStatus.NotStarted;

            if (!(jAppsToken is JObject))
            {
                return commandStatus;
            }

            JObject appsNode = (JObject)jAppsToken;

            // Reset the nodes to report...
            _stateToReport = new Dictionary<string, AppxManagementDataContract.AppReportedState>();

            IDictionary<string, AppInfo> installedApps = await ListAppsAsync();
            DumpInstalledApps(installedApps);

            AppxManagementDataContract.DesiredProperties desiredProperties = AppxManagementDataContract.DesiredProperties.FromJsonObject(appsNode);
            AppUtils.DumpDesiredProperties(desiredProperties);

            ReorderAndValidate(desiredProperties.appDesiredStates, installedApps);

            // Process explicitly defined app blocks...
            foreach (AppxManagementDataContract.AppDesiredState appDesiredState in desiredProperties.appDesiredStates)
            {
                try
                {
                    commandStatus = await ApplyAppDesiredState(this._connectionString, installedApps, appDesiredState);
                }
                catch (Exception)
                {
                    // Catch everything here so that we may continue processing other appDesiredStates.
                    // No reporting to the IoT Hub is needed here because it has already taken place.
                }

                // Should we continue?
                if (commandStatus == CommandStatus.PendingDMAppRestart)
                {
                    break;
                }
            }

            // Process the "?"
            ProcessFullListQuery(installedApps, desiredProperties.appListQuery, _stateToReport);

            // Reset the apps reported node...
            await NullifyReported();

            // Report all collected values...
            foreach (var pair in _stateToReport)
            {
                await ReportAppStatus(pair.Value);
            }

            return commandStatus;
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
            JToken cachedToken = _desiredCache.SelectToken(PropertySectionName);
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
                _desiredCache[PropertySectionName] = desiredValue;
            }
        }

        // IClientPropertyHandler
        public async Task<CommandStatus> OnDesiredPropertyChange(JToken desiredValue)
        {
            UpdateCache(desiredValue);

            // Need to revisit all the desired nodes (not only the changed ones)
            // so that we can re-construct the correct reported list.
            return await ApplyDesiredPropertiesAsync(_desiredCache[PropertySectionName]);
        }

        // IClientPropertyHandler
        public async Task<JObject> GetReportedPropertyAsync()
        {
            IDictionary<string, AppInfo> installedApps = await ListAppsAsync();
            DumpInstalledApps(installedApps);

            JToken cachedToken = _desiredCache.SelectToken(PropertySectionName);

            if (cachedToken == null)
            {
                return null;
            }

            if (!(cachedToken is JObject))
            {
                throw new Error(ErrorCodes.INVALID_DESIRED_CACHE, PropertySectionName + " is expected to be a json object in cache.");
            }

            var desiredProperties = AppxManagementDataContract.DesiredProperties.FromJsonObject((JObject)cachedToken);

            var stateToReport = new Dictionary<string, AppxManagementDataContract.AppReportedState>();

            // Process explicitly defined app blocks...
            foreach (var appDesiredState in desiredProperties.appDesiredStates)
            {
                AppInfo installedAppInfo = FindInstalledAppInfo(installedApps, appDesiredState.packageFamilyName);
                ReportQueriedApp(installedAppInfo, appDesiredState, stateToReport);
            }

            // Process full list query...
            ProcessFullListQuery(installedApps, desiredProperties.appListQuery, stateToReport);

            // Reset the apps reported node...
            await NullifyReported();

            // Report all collected values...
            AppxManagementDataContract.ReportedProperties reportedProperties = new AppxManagementDataContract.ReportedProperties();
            foreach (var pair in stateToReport)
            {
                reportedProperties.appReportedStates.Add(pair.Value);
            }

            return reportedProperties.ToJsonObject();
        }

        private static void ProcessFullListQuery(
            IDictionary<string, AppInfo> installedApps,
            AppxManagementDataContract.AppListQuery appListQuery,
            Dictionary<string, AppxManagementDataContract.AppReportedState> stateToReport
            )
        {
            // Process the "?"
            if (!appListQuery.nonStore && !appListQuery.store)
            {
                // There's nothing to do...
                return;
            }

            foreach (var pair in installedApps)
            {
                AppxManagementDataContract.AppReportedState dummyAppReportedState;
                if (stateToReport.TryGetValue(pair.Value.PackageFamilyName, out dummyAppReportedState))
                {
                    // Already queued to be reported, no need to consider adding it to stateToReport.
                    continue;
                }
                if ((pair.Value.AppSource == AppxManagementDataContract.JsonAppSourceStore && appListQuery.store) ||
                    (pair.Value.AppSource == AppxManagementDataContract.JsonappSourceNonStore && appListQuery.nonStore))
                {
                    AppxManagementDataContract.AppReportedState appReportedState = new AppxManagementDataContract.AppReportedState(
                        pair.Value.PackageFamilyName.Replace('.', '_'),
                        pair.Value.PackageFamilyName,
                        pair.Value.Version,
                        AppUtils.StartUpTypeFromMessage(pair.Value.StartUp),
                        pair.Value.InstallDate,
                        0,
                        null,
                        JsonReport.Report);

                    stateToReport[pair.Value.PackageFamilyName] = appReportedState;
                }
            }
        }

        private string _connectionString;
        private ISystemConfiguratorProxy _systemConfiguratorProxy;
        private IClientHandlerCallBack _callback;
        private JObject _desiredCache;
        private Dictionary<string, AppxManagementDataContract.AppReportedState> _stateToReport;
    }
}
