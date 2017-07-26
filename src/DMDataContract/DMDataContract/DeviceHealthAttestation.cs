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

namespace Microsoft.Devices.Management.DMDataContract
{
    /// <summary>
    /// Data contract between DM-client and IoTHub.
    /// This class describes all the device health attestation related data structures used in the 
    /// device twin and telemetry messages between the DM client and IoTHub.
    /// </summary>
    public class DeviceHealthAttestationDataContract
    {
        //
        // JSON Property Name
        //
        public static readonly string JsonSectionName = "deviceHealthAttestation";

        /// <summary>
        /// Desired Properties
        /// </summary>
        public struct DesiredProperty
        {
            /// <summary>
            /// Health Attestion Service endpoint.
            /// If empty, the default Microsoft Health Attestation Service endpoint will be used.
            /// </summary>
            public string Endpoint;

            /// <summary>
            /// Reporting interval in seconds
            /// If the value is negative, health attestation will be disabled.
            /// If the value is zero, health attestation will only be performed on startup.
            /// If the value is positive, health attestation will be performed by the specified interval.
            /// </summary>
            public int ReportIntervalInSeconds;
        }

        /// <summary>
        /// Reported Properties
        /// </summary>
        public struct ReportedProperty
        {
            /// <summary>
            /// Status of last report
            /// </summary>
            public string status;
        }


        /// <summary>
        /// GetCertificate Direct Method Name
        /// </summary>
        public static readonly string GetReportMethodName = "windows.deviceHealthAttestationGetReport";
        /// <summary>
        /// Data Structure for GetReport direct method.
        /// </summary>
        public struct GetReportMethodParam
        {
            /// <summary>
            /// The Nonce value
            /// </summary>
            public string Nonce;
        }

        /// <summary>
        /// ReportNow Direct Method Name
        /// </summary>
        public static readonly string ReportNowMethodName = "windows.deviceHealthAttestationReportNow";
        // Parameter not used

        //
        // Device to Cloud Messages
        //

        /// <summary>
        /// DHA-Nonce Message Tag
        /// </summary>
        public static readonly string NonceRequestTag = "DHA-Nonce";
        /// <summary>
        /// DHA-Nonce Message Body
        /// </summary>
        // Not used

        /// <summary>
        /// DHA-HealthReport Message Tag
        /// </summary>
        public static readonly string HealthReportTag = "DHA-HealthReport";

        /// <summary>
        /// DHA-HealthReport Message Body
        /// </summary>
        public struct HealthReport
        {
            /// <summary>
            /// The correlation ID associated with this report.
            /// </summary>
            public string CorrelationId;

            /// <summary>
            /// The health certificate issued by the Health Attestation Service.
            /// </summary>
            public string HealthCertificate;
        }
    }
}
