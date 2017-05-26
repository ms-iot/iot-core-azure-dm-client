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

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading.Tasks;
using Microsoft.Devices.Management.DMDataContract;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using Windows.Storage;
using Windows.System.Threading;

namespace Microsoft.Devices.Management
{
    internal class DeviceHealthAttestationHandler : IClientPropertyHandler, IClientDirectMethodHandler
    {
        private static readonly string ApplicationDataContainerName = "Microsoft.Devices.Management.DeviceHealthAttestation";
        private static readonly string LastHealthReportTimeTag = "LastHealthReport";

        public DeviceHealthAttestationHandler(IClientHandlerCallBack callback, ISystemConfiguratorProxy systemConfiguratorProxy)
        {
            this._systemConfiguratorProxy = systemConfiguratorProxy;
            this._callback = callback;
            this._reportInterval = TimeSpan.Zero;

            // Create the settings container
            this._container = ApplicationData.Current.LocalSettings.CreateContainer(ApplicationDataContainerName, ApplicationDataCreateDisposition.Always);
        }

        public IReadOnlyDictionary<string, Func<string, Task<string>>> GetDirectMethodHandler()
        {
            return new Dictionary<string, Func<string, Task<string>>>()
                {
                    { DeviceHealthAttestationDataContract.GetReportMethodName , GetReportHandlerAsync },
                    { DeviceHealthAttestationDataContract.ReportNowMethodName , ReportNowHandlerAsync }
                };
        }

        public string PropertySectionName
        {
            get
            {
                return DeviceHealthAttestationDataContract.JsonSectionName;
            }
        }

        public void OnDesiredPropertyChange(JObject value)
        {
            VerifyHealthAsync(value); // fire and forget
        }

        public async Task<JObject> GetReportedPropertyAsync()
        {
            return await Task.FromResult(JObject.FromObject(_reportedProperty));
        }

        private async Task VerifyHealthAsync(JObject value)
        {
            try
            {
                CancelScheduledReporting();

                var properties = value.ToObject<DeviceHealthAttestationDataContract.DesiredProperty>();
                _reportInterval = TimeSpan.FromSeconds(properties.ReportIntervalInSeconds);

                if (_reportInterval < TimeSpan.Zero)
                {
                    ReportStatus("Reporting Disabled");
                    return;
                }
                else
                {
                    // Execute verify health
                    var request = new Message.DeviceHealthAttestationVerifyHealthRequest();
                    request.HealthAttestationServerEndpoint = properties.Endpoint;
                    await this._systemConfiguratorProxy.SendCommandAsync(request);
                    // Report status OK
                    ReportStatus("VerifyHealth Success");

                    if (_reportInterval == TimeSpan.Zero)
                    {
                        await RequestNonceAsync();
                    }
                    else
                    {
                        ScheduleNextReportingAsNeeded();
                    }
                }
            }
            catch (Exception e)
            {
                Debug.WriteLine($"VerifyHealth failed: {e}");
                ReportStatus($"VerifyHealth failed: {e.Message}");
            }
        }

        private void CancelScheduledReporting()
        {
            lock(this)
            {
                if (this._timer != null)
                {
                    this._timer.Cancel();
                    this._timer = null;
                }
            }
        }

        private void ScheduleNextReportingAsNeeded()
        {
            lock(this)
            {
                CancelScheduledReporting();
                // Only schedule timer if timespan is +ve.
                if (_reportInterval > TimeSpan.Zero)
                {
                    var nextReportTime = GetLastHealthReportTime() + _reportInterval;
                    var delay = nextReportTime - DateTime.UtcNow;
                    if (delay < TimeSpan.Zero)
                    {
                        delay = TimeSpan.Zero;
                    }
                    this._timer = ThreadPoolTimer.CreateTimer(ScheduledReportHandler, delay);
                }
            }
        }

        private void ScheduledReportHandler(ThreadPoolTimer timer)
        {
            // Fire and forget
            RequestNonceAsync();
        }

        private async Task RequestNonceAsync()
        {
            try
            {
                // Schedule next reporting
                SetLastHealthReportTime(DateTime.UtcNow);
                ScheduleNextReportingAsNeeded();

                var properties = new Dictionary<string, string>();
                properties.Add("MessageType", DeviceHealthAttestationDataContract.NonceRequestTag);
                await _callback.SendMessageAsync("{}", properties);
                await ReportStatus("Nonce requested");
            }
            catch(Exception e)
            {
                Debug.WriteLine($"Request nonce failed with exception: {e}");
                await ReportStatus($"Request nonce failed with exception: {e.Message}");
            }
        }

        private async Task ReportStatus(string status)
        {
            _reportedProperty.status = status;
            await _callback.ReportPropertiesAsync(PropertySectionName, JObject.FromObject(_reportedProperty));
        }

        private async Task GetReportHandlerAsync(Message.DeviceHealthAttestationGetReportRequest request)
        {
            try
            {
                var response = await _systemConfiguratorProxy.SendCommandAsync(request) as Message.DeviceHealthAttestationGetReportResponse;

                var report = new DeviceHealthAttestationDataContract.HealthReport()
                {
                    CorrelationId = response.CorrelationId,
                    HealthCertificate = response.HealthCertificate
                };
                var json = JsonConvert.SerializeObject(report);

                var properties = new Dictionary<string, string>();
                properties.Add("MessageType", DeviceHealthAttestationDataContract.HealthReportTag);
                await _callback.SendMessageAsync(json, properties);
                await ReportStatus("Reported");
            }
            catch (Exception e)
            {
                Debug.WriteLine($"Get report failed with exception: {e}");
                await ReportStatus($"Get report failed with exception: {e.Message}");
            }
        }

        private Task<string> ReportNowHandlerAsync(string jsonParam)
        {
            Debug.WriteLine("DeviceHealthAttestationHandler.ReportNowHandlerAsync");

            // Submit the work and return immediately.
            RequestNonceAsync();

            var response = new { response = "succeeded", reason = "" };
            return Task.FromResult(JsonConvert.SerializeObject(response));
        }

        private Task<string> GetReportHandlerAsync(string jsonParam)
        {
            Debug.WriteLine("DeviceHealthAttestationHandler.GetReportHandlerAsync");

            var response = new { response = "succeeded", reason = "" };
            try
            {
                var param = JsonConvert.DeserializeObject<DeviceHealthAttestationDataContract.GetReportMethodParam>(jsonParam);
                var request = new Message.DeviceHealthAttestationGetReportRequest();
                request.Nonce = param.Nonce;
                // Submit the work and return immediately.
                GetReportHandlerAsync(request);
            }
            catch (Exception e)
            {
                response = new { response = "rejected:", reason = e.Message };
            }

            return Task.FromResult(JsonConvert.SerializeObject(response));
        }

        private DateTime GetLastHealthReportTime()
        {
            object value;
            if (_container.Values.TryGetValue(LastHealthReportTimeTag, out value) && value is long)
            {
                return DateTime.FromBinary((long)value);
            }
            return new DateTime();
        }

        private void SetLastHealthReportTime(DateTime value)
        {
            _container.Values[LastHealthReportTimeTag] = value.ToBinary();
        }

        private ISystemConfiguratorProxy _systemConfiguratorProxy;
        private IClientHandlerCallBack _callback;
        private ApplicationDataContainer _container;
        private ThreadPoolTimer _timer;
        private DeviceHealthAttestationDataContract.ReportedProperty _reportedProperty;
        private TimeSpan _reportInterval;
    }
}
