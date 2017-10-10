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

using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Text;

namespace Microsoft.Devices.Management.DMDataContract
{
    public enum JsonReport
    {
        Report,
        Unreport
    }

    public enum AppDesiredAction
    {
        Undefined,
        Install,
        Uninstall,
        Query,
        Unreport
    }

    public enum AppxStartUpType
    {
        None = 0,
        Foreground = 1,
        Background = 2
    }

    public class AppxManagementDataContract
    {
        public const string NotFound = "<not found>";
        public const string SectionName = "apps";

        public const string JsonQueryApps = "?";

        public const string JsonStartUp = "startUp";
        public const string JsonStartUpNone = "none";
        public const string JsonStartUpForeground = "foreground";
        public const string JsonStartUpBackground = "background";

        public const string JsonPkgFamilyName = "pkgFamilyName";
        public const string JsonVersion = "version";
        public const string JsonInstallDate = "installDate";
        public const string JsonErrorCode = "errorCode";
        public const string JsonErrorMessage = "errorMessage";

        public const string JsonAppSourceStore = "AppStore";
        public const string JsonappSourceNonStore = "NonStore";

        public const string VersionNotInstalled = "not installed";

        public const string JsonAppxSource = "appxSource";
        public const string JsonDepsSources = "depsSources";
        public const string JsonCertSource = "certSource";
        public const string JsonCertStore = "certStore";

        public static AppxStartUpType StartUpTypeFromString(string value)
        {
            AppxStartUpType startUp = AppxStartUpType.None;
            switch (value)
            {
                case JsonStartUpForeground:
                    startUp = AppxStartUpType.Foreground;
                    break;
                case JsonStartUpBackground:
                    startUp = AppxStartUpType.Background;
                    break;
            }
            return startUp;
        }

        public static string StartUpTypeToString(AppxStartUpType startUpType)
        {
            switch (startUpType)
            {
                case AppxStartUpType.Foreground:
                    return JsonStartUpForeground;
                case AppxStartUpType.Background:
                    return JsonStartUpBackground;
            }
            return JsonStartUpNone;
        }

        public class AppListQuery
        {
            public const string JsonStore = "store";
            public const string JsonNonStore = "nonStore";

            public bool store;
            public bool nonStore;

            public static AppListQuery FromJsonObject(JObject obj)
            {
                AppListQuery query = new AppListQuery();
                query.store = Utils.GetBool(obj, JsonStore, false);
                query.store = Utils.GetBool(obj, JsonNonStore, false);
                return query;
            }
        }

        public class AppDesiredState
        {
            public string packageFamilyId;
            public string packageFamilyName;
            public AppDesiredAction action;
            public Version version;
            public AppxStartUpType startUp;
            public string appxSource;
            public string depsSources;
            public string certSource;
            public string certStore;
            public int errorCode;
            public string errorMessage;

            public AppDesiredState(string packageFamilyId)
            {
                this.packageFamilyId = packageFamilyId;
                this.action = AppDesiredAction.Undefined;
            }

            public static AppDesiredState FromJsonObject(string packageFamilyId, JToken jToken)
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

                        if (appProperty.Name == JsonPkgFamilyName)
                        {
                            desiredState.packageFamilyName = appProperty.Value.ToString();
                        }
                        else if (appProperty.Name == JsonStartUp)
                        {
                            desiredState.startUp = AppxManagementDataContract.StartUpTypeFromString(appProperty.Value.ToString());
                        }
                        else if (appProperty.Name == JsonVersion)
                        {
                            string value = appProperty.Value.ToString();
                            if (value == JsonQueryApps)
                            {
                                desiredState.action = AppDesiredAction.Query;
                            }
                            else if (value == AppxManagementDataContract.VersionNotInstalled)
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
                                    desiredState.errorCode = ErrorCodes.INVALID_DESIRED_VERSION;
                                    desiredState.errorMessage = "Invalid desired version format!";
                                }
                            }
                        }
                        else if (appProperty.Name == JsonAppxSource)
                        {
                            desiredState.appxSource = appProperty.Value.ToString();
                        }
                        else if (appProperty.Name == JsonDepsSources)
                        {
                            desiredState.depsSources = appProperty.Value.ToString();
                        }
                        else if (appProperty.Name == JsonCertSource)
                        {
                            desiredState.certSource = appProperty.Value.ToString();
                        }
                        else if (appProperty.Name == JsonCertStore)
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
                        desiredState.errorCode = ErrorCodes.INVALID_DESIRED_PKG_FAMILY_ID;
                        desiredState.errorMessage = "Invalid desired package family id value!";
                    }
                }

                return desiredState;
            }

            public JObject ToJsonstring()
            {
                throw new Exception("AppxManagementDataContract.AppDesiredState.ToJsonString() Not implemented");
            }
        }

        public class DesiredProperties
        {
            public List<AppDesiredState> appDesiredStates;
            public AppListQuery appListQuery;

            public DesiredProperties(List<AppDesiredState> states, AppListQuery query)
            {
                appDesiredStates = states;
                appListQuery = query;
            }

            public static DesiredProperties FromJsonObject(JObject appsNode)
            {
                List<AppDesiredState> appDesiredStates = new List<AppxManagementDataContract.AppDesiredState>();
                AppListQuery appListQuery = new AppListQuery();

                foreach (JProperty property in appsNode.Children())
                {
                    // Logger.Log("Property Name = " + property.Name, LoggingLevel.Verbose);
                    if (String.IsNullOrEmpty(property.Name))
                    {
                        continue;
                    }

                    if (property.Name == JsonQueryApps)
                    {
                        appListQuery = AppListQuery.FromJsonObject((JObject)property.Value);
                    }
                    else
                    {
                        appDesiredStates.Add(AppDesiredState.FromJsonObject(property.Name, property.Value));
                    }
                }

                return new DesiredProperties(appDesiredStates, appListQuery);
            }

            public JObject ToJsonstring()
            {
                throw new Exception("AppxManagementDataContract.DesiredProperties.ToJsonString() Not implemented");
            }
        }

        public class AppReportedState
        {
            public string packageFamilyId;
            public string packageFamilyName;
            public string version;
            public AppxStartUpType startUp;
            public string installDate;
            public int errorCode;
            public string errorMessage;
            public JsonReport report;

            public AppReportedState(
                string packageFamilyId,
                string packageFamilyName,
                string version,
                AppxStartUpType startUp,
                string installDate,
                int errorCode,
                string errorMessage,
                JsonReport report)
            {
                this.packageFamilyId = packageFamilyId;
                this.packageFamilyName = packageFamilyName;
                this.version = version;
                this.startUp = startUp;
                this.installDate = installDate;
                this.errorCode = errorCode;
                this.errorMessage = errorMessage;
                this.report = report;
            }

            public static AppReportedState FromJsonObject(JObject json)
            {
                throw new Exception("AppxManagementDataContract.DesiredProperties.ToJsonString() Not implemented");
            }

            public JObject ToJsonObject()
            {
                JObject jObj = new JObject();
                if (report == JsonReport.Report)
                {
                    JObject jPkgFamilyObj = new JObject();

                    jPkgFamilyObj.Add(JsonPkgFamilyName, packageFamilyName);
                    jPkgFamilyObj.Add(JsonVersion, version);
                    jPkgFamilyObj.Add(JsonStartUp, StartUpTypeToString(startUp));
                    jPkgFamilyObj.Add(JsonInstallDate, installDate);
                    if (errorCode != 0)
                    {
                        jPkgFamilyObj.Add(JsonErrorCode, errorCode);
                        jPkgFamilyObj.Add(JsonErrorMessage, errorMessage);
                    }

                    jObj.Add(packageFamilyId, jPkgFamilyObj);
                }
                else
                {
                    jObj.Add(packageFamilyId, null);
                }

                return jObj;
            }

            public string ToJsonString()
            {
                return ToJsonObject().ToString();
            }
        }

        public class ReportedProperties
        {
            public List<AppReportedState> appReportedStates;

            public ReportedProperties()
            {
                appReportedStates = new List<AppReportedState>();
            }

            public static ReportedProperties FromJsonObject(JObject json)
            {
                throw new Exception("AppxManagementDataContract.DesiredProperties.ToJsonString() Not implemented");
            }

            public JObject ToJsonObject()
            {
                JObject obj = new JObject();
                foreach (var reportedState in appReportedStates)
                {
                    obj.Add(reportedState.packageFamilyId, reportedState.ToJsonObject());
                }
                return obj;
            }
        }
    }
}