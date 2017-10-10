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
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System;
using System.Diagnostics;
using System.Threading.Tasks;
using Windows.Foundation.Diagnostics;

namespace Microsoft.Devices.Management
{
    static class Constants
    {
        public const string ETWGuid = "D198EE7D-C5F1-4F5F-95BE-A2EE6FA45897";   // Use this guid with xperf, the CSP, etc.
        public const string ETWChannelName = "AzureDM";

        public const string JsonValueUnspecified = "<unspecified>";
    }

    public class Error : Exception
    {
        const string SubSystemString = "errSubSystem";
        const string ErrorCodeString = "errCode";
        const string ErrorContextString = "errContext";

        public Error() { }

        public Error(int code, string message) : base(message)
        {
            this.HResult = code;
        }

        public Error(Message.ErrorSubSystem subSystem, int code, string message) : base(message)
        {
            this.HResult = code;
            this.SubSystem = subSystem;
        }

        public JObject ToJson()
        {
            JObject jErrorDetails = new JObject();
            jErrorDetails.Add(SubSystemString, new JValue(SubSystem.ToString()));
            jErrorDetails.Add(ErrorCodeString, new JValue(this.HResult));
            jErrorDetails.Add(ErrorContextString, new JValue(base.Message));
            return jErrorDetails;
        }

        public override string ToString()
        {
            return JsonConvert.SerializeObject(ToJson());
        }

        public Message.ErrorSubSystem SubSystem { get; private set; }
    }

    public class StatusSection
    {
        const string SectionName = "lastChange";
        const string TimeString = "time";
        const string StateString = "state";
        const string Refreshing = "refreshing";

        public enum StateType
        {
            Pending,
            Completed,
            Failed
        }

        public StateType State { get; set; }
        public Error TheError { get; set; }

        public StatusSection(StateType state)
        {
            _dateTime = DateTime.Now;
            State = state;
        }

        public StatusSection(StateType state, Error error)
        {
            _dateTime = DateTime.Now;
            State = state;
            TheError = error;
        }

        public JProperty AsJsonPropertyRefreshing()
        {
            return new JProperty(SectionName, new JValue(Refreshing));
        }

        public JProperty AsJsonProperty()
        {
            JObject jStatusObject = new JObject();
            jStatusObject.Add(TimeString, new JValue(_dateTime));
            jStatusObject.Add(StateString, new JValue(State.ToString().ToLower()));
            if (TheError != null)
            {
                jStatusObject.Merge(TheError.ToJson());
            }

            return new JProperty(SectionName, jStatusObject);
        }

        public JObject AsJsonObject()
        {
            JProperty jStatusProperty = AsJsonProperty();
            JObject jStatusObject = new JObject();
            jStatusObject.Add(jStatusProperty);
            return jStatusObject;
        }

        public override string ToString()
        {
            return JsonConvert.SerializeObject(AsJsonProperty().Value);
        }

        DateTime _dateTime;
    }

    static class Helpers
    {
        public static async Task EnsureErrorsLogged(IClientHandlerCallBack _callback, string sectionName, Func<Task> action)
        {
            try
            {
                await action();
            }
            catch (Error ex)
            {
                StatusSection status = new StatusSection(StatusSection.StateType.Failed, ex);

                Logger.Log(status.ToString(), LoggingLevel.Error);
                await _callback.ReportStatusAsync(sectionName, status);
            }
            catch (Exception ex)
            {
                Error e = new Error(ErrorSubSystem.Unknown, ex.HResult, ex.Message);
                StatusSection status = new StatusSection(StatusSection.StateType.Failed, e);

                Logger.Log(status.ToString(), LoggingLevel.Error);
                await _callback.ReportStatusAsync(sectionName, status);
            }
        }
    }

    enum CommandStatus
    {
        NotStarted,
        Committed,
        PendingDMAppRestart
    }

    public enum SettingsPriority
    {
        Unknown,
        Local,
        Remote
    }

    public class Logger
    {
        public static void Log(string message, LoggingLevel level)
        {
            /*
                You can collect the events generated by this method with xperf or another
                ETL controller tool. To collect these events in an ETL file:

                xperf -start MySession -f MyFile.etl -on Constants.ETWGuid
                (call LogError())
                xperf -stop MySession

                After collecting the ETL file, you can decode the trace using xperf, wpa,
                or tracerpt. For example, to decode MyFile.etl with tracerpt:

                tracerpt MyFile.etl
                (generates dumpfile.xml)
            */

            using (var channel = new LoggingChannel(Constants.ETWChannelName, null /*default options*/, new Guid(Constants.ETWGuid)))
            {
                Debug.WriteLine("[" + Constants.ETWChannelName + "]    [" + level.ToString() + "]    " + message);
                channel.LogMessage(message, level);
            }
        }
    }

    static class AsyncHelper
    {
        public static async void FireAndForget(this Task task)
        {
            try
            {
                await task;
            }
            catch (Exception)
            {
                // log errors
            }
        }
    }

    public static class PolicyHelpers
    {
        public static string SourcePriorityFromPolicy(Message.Policy policy)
        {
            if (policy == null || policy.sourcePriorities == null || policy.sourcePriorities.Count == 0)
            {
                return PolicyDataContract.JsonUnknown;
            }

            if (policy.sourcePriorities[0] == Message.PolicySource.Local)
            {
                return PolicyDataContract.JsonLocal;
            }
            else if (policy.sourcePriorities[0] == Message.PolicySource.Remote)
            {
                return PolicyDataContract.JsonRemote;
            }
            return PolicyDataContract.JsonUnknown;
        }

        public static SettingsPriority SettingsPriorityFromString(string s)
        {
            switch (s)
            {
                case PolicyDataContract.JsonLocal:
                    return SettingsPriority.Local;
                case PolicyDataContract.JsonRemote:
                    return SettingsPriority.Remote;
            }
            return SettingsPriority.Unknown;
        }

        public static readonly Message.PolicySource[] PriorityLocal = {
            Message.PolicySource.Local, // local takes precedence
            Message.PolicySource.Remote
        };

        public static readonly Message.PolicySource[] PriorityRemote = {
            Message.PolicySource.Remote, // remote takes precedence
            Message.PolicySource.Local
        };
    }

}