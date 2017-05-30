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

using Microsoft.Azure.Devices;
using Microsoft.Azure.Devices.Client;
using Microsoft.Azure.Devices.Shared;
using Microsoft.Devices.Management.DMDataContract;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace DHA_test
{
    [TestClass]
    public class DeviceHealthAttestationServerTest
    {
        static string IotHubOwnerConnectionString = "<Your IotHubOwner Connection string>";
        static string TestDeviceId = "DHA-testClient";

        public TestContext TestContext { get; set; }


        [TestMethod]
        public void NonceRequestEnsureNonEmpty()
        {
            Func<Task> asyncMethod = async () =>
            {
                var connectionString = await GetTestDeviceConnectionString();
                var deviceClient = DeviceClient.CreateFromConnectionString(connectionString, Microsoft.Azure.Devices.Client.TransportType.Mqtt);

                var nonceReady = new AutoResetEvent(/*init=*/false);
                string actualNonce = string.Empty;
                MethodCallback GetReportMethodHandler = (MethodRequest methodRequest, object userContext) =>
                {
                    var data = JsonConvert.DeserializeObject<DeviceHealthAttestationDataContract.GetReportMethodParam>(methodRequest.DataAsJson);
                    actualNonce = data.Nonce;
                    TestContext.WriteLine($"Got Nonce:{actualNonce}");
                    nonceReady.Set();

                    var response = new { response = "succeeded", reason = "" };
                    return Task.FromResult(new MethodResponse(Encoding.UTF8.GetBytes(JsonConvert.SerializeObject(response)), 0));
                };
                await deviceClient.SetMethodHandlerAsync(DeviceHealthAttestationDataContract.GetReportMethodName, GetReportMethodHandler, null);

                var properties = new Dictionary<string, string>();
                properties.Add("MessageType", DeviceHealthAttestationDataContract.NonceRequestTag);
                await deviceClient.SendMessageAsync("", properties);

                Assert.IsTrue(nonceReady.WaitOne(5000));

                var nonceFromTable = await NonceCloudTable.Instance.GetNonceForDeviceAsync(TestDeviceId);
                await NonceCloudTable.Instance.DeleteNonceForDeviceAsync(TestDeviceId);
                Assert.AreEqual(nonceFromTable, actualNonce);
            };

            asyncMethod().Wait();
        }

        [TestMethod]
        public void ReportRequestTest()
        {
            string correlationId = Guid.NewGuid().ToString();
            string ValidNonce = "44BA2C8243B22BB30528CDA00A7444F2";
            string ValidHealthCertificate = "PD94bWwgdmVyc2lvbj0iMS4wIiBlbmNvZGluZz0idXRmLTgiPz48SGVhbHRoQ2VydGlmaWNhdGVWYWxpZGF0aW9uUmVxdWVzdCBQcm90b2NvbFZlcnNpb249IjMiIHhtbG5zPSJodHRwOi8vc2NoZW1hcy5taWNyb3NvZnQuY29tL3dpbmRvd3Mvc2VjdXJpdHkvaGVhbHRoY2VydGlmaWNhdGUvdmFsaWRhdGlvbi9yZXF1ZXN0L3YzIj48Q2xhaW1zPkFRQUFBQVFBQUFCMUFBQUFCZ0VBQUFBREFBRC9WRU5IZ0JnQUlnQUx2MWFqaFhHYU9yVGp6VlBFelcrSUs3ckx4dU1QeXhwMGVOa3lsNzZ1THowQUVFUzZMSUpEc2l1ekJTak5vQXAwUlBJQUFBQUFkYituUFFmT2ZqTFAyYkpOQVNwZStheTNMbmErQUFBQUFRQUxBLy8vL3dBVWVGakliMFZES1p5RHRBOHJXUVVtaUhNSlkvSUFGQUFFQVFDZXFGbDdMTFRBUytrUUdyT0liVW0zKzRyc0JGeHpSUVFJUmVSMXdTU3lyLzNPSStoczZPZDRUT0YyZUhzUXZkZVRLdE1rMWtqQWtpRzlqcmh1Uzdna1ZqZnpUOHBNMW52QmM4VTdTc09OaWhHZmJIeFdHdDhQdXNtQTlPZkVla1JUZng0eTBWSHdCYUl5UE91Mzg5ZkxHV29BTFFiendlb2sxQmp1V2NFRUhVRUdTRCtRNHQvV3o0UUpyV0FUeGkwYVZjOGowZDZNVVhwOXpwUDNRMk05VW1rKzQ3Z3J5aGhDdFJ3UUtVdDBNWkVDdlEzRzUrakFMYmZyUUI3K3ArN1RvU0JaZmZydk8vUkQ5ZUNnb0lueEVpUlZNbkpKcWpkTE1QZnZ3UG9xK0NsR3UvbEg4cktQeFM0YnVGZEt2bzF6WHhxVSsyRlRxbWNLWWxBcC9nQTlHRUxJWHc1cTFuaFpxQWM1blhxQTNQTGZVTlhNMStZeUcrZ2J5Rk44b1lNdUVvNUpkbGR6d0xrU2xrNkhYYnJKcm4yMUd1ZVVNTUlTeVBKbTR2ZW9VRDFGalA1VnpBUHFIMFEvRldLKzdJMzFISFhoU3AvUG1uSTBvVDhaam5scFBVV00vbFhNQStvZlJEOFZZcjdzamZVY2RlRktuOCthY2pTaFB4bU9lV2s5Ull6K1Zjd0Q2aDlFUHhWaXZ1eU45UngxNFVxZno1cHlOS0UvR1k1NWFhd3QrTGpzVVgwaEFMbWlkUldkU0pWeW1oUVR4SDlCdzIxTG9kbVNmT0Z5UFVXTS9sWE1BK29mUkQ4VllyN3NqZlVjZGVGS244K2FjalNoUHhtT2VXbjIxYi9wWEVYd2t1dUV4OVhwdTE5S0JsU1dFUDIrU2tITERzVEFQMFVEQ0FBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQS9tNlBJUkRWMVRrMXllZlc5cjl5SlppMVVGbGFxOXh1VDlMczN6RVBtQWczMHhka245ejBaME9lb3E2QjFicFBzeWlHekprd0R0UmI4Y0E0YmtYczlTN3MxdTZOYmpET21DYjI0bUQ4dk5RbSt6Z2lxbGVpaTgrTnY1S2VEaFF0ekFLQ3dqenROeGlNTk9hMEY0Rm5ZSGVOVHhMTC9VL3ZmdkpJTmZWcUZjQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQVAvLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy93QUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBPC9DbGFpbXM+PEhlYWx0aENlcnRpZmljYXRlQmxvYj43N3UvUEQ5NGJXd2dkbVZ5YzJsdmJqMGlNUzR3SWlCbGJtTnZaR2x1WnowaWRYUm1MVGdpUHo0OFQzQmhjWFZsU0dWaGJIUm9RMlZ5ZEdsbWFXTmhkR1VnZUcxc2JuTTZlSE5rUFNKb2RIUndPaTh2ZDNkM0xuY3pMbTl5Wnk4eU1EQXhMMWhOVEZOamFHVnRZU0lnZUcxc2JuTTZlSE5wUFNKb2RIUndPaTh2ZDNkM0xuY3pMbTl5Wnk4eU1EQXhMMWhOVEZOamFHVnRZUzFwYm5OMFlXNWpaU0lnVm1WeWMybHZiajBpTVNJZ2VHMXNibk05SW1oMGRIQTZMeTl6WTJobGJXRnpMbTFwWTNKdmMyOW1kQzVqYjIwdmQybHVaRzkzY3k5elpXTjFjbWwwZVM5b1pXRnNkR2hqWlhKMGFXWnBZMkYwWldKc2IySXZkakVpUGp4RmJtTnllWEIwWldSSVpXRnNkR2hEWlhKMGFXWnBZMkYwWlQ1UmFYQTVTa3RuZFdoVFRGaEJlRlJWZWtwak1FVlNWVkJqYUZkWGJHVmxZbUZFU1U0MVpEQnFkemRGS3pGWFFqUjZRVTlpYUZWRmVXWTBjWGxpUTNadVZUQndReXRyVEZZdk5IRmlLMGQxUTBFMFoxRkRlbWhsYmsxMlYydE5SV2RzWTNsNmQxVnViVFU0VjFnNVptVnZSSEEwZERkM1FqSm1NSFJTZGs1MFdGUlFkVGcyVjBwRU5VTktTMFoxVERSdGNDdFlSRFF2U1V0MlQwZHFlVEZ0Y2pRMFFVNUdkelp2WTA5NU9IUXJWRmRoUkZCWEt6WkpXREJWUkhGb00yaEJaazV2TDFod2JHeFpWbmQ2V2pKSlYxWm9VMWhRT0hacldFbE5XVEpETDI5Uk9TOTFjVXRNYW1SQ1RYRTROV2xvWkRGMVUybG9ZbHAwSzNaTFRHRnZjVFpGV2xGSGMwWnhjV3hKVm5wMk4ycExhMmcxY1hGWmR6TlhOSG93VEdrMFpqUnJOamRxSzNkcmJYVTJTVzkwT1dRMlVtTkJRekZ2WTFwRmVIZG5ORlJxVFU1VlFYUm5kVmt4YlhKa1ptRllVMnB4WVZsbFJrUk9WbUZQY0V0Wk5XSTFObEZLYm05d2R6bEVaRmxUV21reFRuaHdWRWxMWTJ0Rk1XdG9jMnN6Y2tOdU0yVXpPRzlMYWk5dlozbEdTWEZYTmpGUGR6TmxaMUpMUkZoNVN6TlZaVTh6WkVnMk1GQTJZekptTUZoMFdFcHpNVUZ2THpsbU9ISXhaamg1WkUxUlF6QlpSa001YTBSQ1YyWklVVTFZV1hSclFTdHlOVEZ6VlhWUU0yTm9ZM1ppTjBrNFZHVjZiV0pCTTJOb2JHNXhUMkpZU0UxRE9GSm1XVkZHZVM5M1RreGxkRU0yYzFONWJHZExPRVo1ZHpjM09WZEJPVGRRVjBsek5ESmxNMVZDVldWeVNIVlpXVVZETTFKMFoyZG5aR2hPZFV0b1QxUnNXalpUWTJjeVptUk5NRFpxWnpSb1NUTnpOR2w2T0ZSb1pHaGxVVEZwZFZCQ2RuTkVNRkk0Yms1NE4zaFJXbGhYZGxoNGVsbHVNRlpDUVcxMVJHRnJiSFp3UXpoNGRHTndXa1ZCUVhrd2RYTjRlRmRCVkdwMlVVNVZRbWxoZEdWU2IwSlRNR1l3WlhSdVMxUjZTRkYwV0ZJMFFqZzJlWGh2U1hOUU9VWkhhMjRyYlRGbVJYQTRhMHBNTDFWbFUybFZjbGhMVDFVeUswNXZRMU5hVlhnMVVHeENjVVZUVFVkbFZVVm1RVFZ4UnpCa1luUjBSbE41YkdodlUwTlpTMHAwV2xVeVNsbHZUMWR6ZVZGT1kybE1lUzlWZUhCb1pHNVNXWFl3TTBGcWRFczBkMnhzZWxKcFJtNTFXamt5WWxrMGRERTRkWGgxZERSUFFYbGtiell5WW01UWRVWnNRMjFxVUVoSk1rRm9hVkEwUzJreFRFRjBZWGxqVGpWRFRXcFNTV1IxTWxjd2VuZHFkbUU0TjJKbFlrdFJTRE5DWWsxRFdYWTFTVkpvU2pOUVF5OTRLM1JyWjBoV01FNTRhV3BJV0hvckwxVkhNblZXTW5Cb2NIWk1NVlZ3WjNack1VUlNiazkzUWxGUWNYbGxMM1I1Um5SdlYxRnZWVWhzZVZCS1ZFZGpSM001TTNGbk5WaENabVpoWkZGNGFIQkNOR0ZQT0hVeE4yOUtNV05XUjBaSVVtMXJTV0ZITWxaaVRWVTFWSEZSVkZZeVVESmhSMDlwZG1adFFsRkJaSFprYjFWb1oyWmlTMlJaYlRsaVJHaE9kMmQ0T1ZFcldFWkVia042VTFwR2JuVmphVGxVUmpRMVlXdHJLMkZIZDI5QlJsSkdjVGxpWW1JMVRVVllUM2hFWlZZclZEY3paMEY1TDFoTFlreHZZMjlDVVRacU1uUlhSbWx4WlZZMmQyMXRVVTh5VmtKS2FHOUpWR0pMUjBNMEt6aHpkRVJZUkM5V2VHeDZOVzlXYVdKT2RGcHlVRXhUVkhsWFdHVjJWVGh4ZWpZelRYb3ZWbEJzY1Zkb2JDdEhjbmxDWkdsblpsSTVXQzloWms5blV5OUpORmgzWkVKek9UaE1kMGRHYW1wME9YUXZaRU5pVDNkb1prdHVjV1JqYkRFdlpqQXlWRkJwY2tWeVprWjZOM1kzTjBaWGVHNUxlVWt6YTA1TlFVVXlLMU5WZEZvMk5ISmFlRU16Y0hwdlIxUndXazlKT0VKQ2JXeDJTbTQzY3pWaGMydEJURGhCYzA5Tk5uUlpVVVkyVmtremNtUlVkWFpCWnlzMmIxaEljbW94UXpOMlVFZHlaSEZ1YlVGbFFrNUJaRXB4THk5MlowSmhPV0ZKWlhGeVQxVnVRM0JOUkM5blRVOTVRMW94TjFZNGNrWXpia1FyVUd0d1lpdGxXR3ByUW1oRk4yTlhOalZLVURVMlkwTnFlRmhTZFVOMWExQTRUR0pJYTFKdmVVRlhVV3B1ZFVndk9VbDBWWEJqU1hoR2RuTmpRMjFXY1Vkb05qVjNVbkJyU1RCaFlXSkRNbk5RV2pkc2JVVnRVbFZTYkdSYVFqQlVhelpsVUc4MVJVRjNWbkprV0VkYWJUVllVVlZXYkUxdGNGZFpSazVSUkc1Qk5EQnhRVWx4V21NMU5HSkJlRXd4YnpSek5UZElNbVJhY1RCdVpEa3plbTFRY25KcWJYZFRZbmhJTXprMk16Sk1ZVm8xZEZKbVFYVnROMFUyYWxaT1FtUkRUVWs1VUhCMlVuTkthM05DZVZCaVVEWk9RV3REWVRoMGVVOHJOMFZwVm5JM1FqQnZUVzFoVVV4S1dFRkxlRXhDWW1aMFIwVjNNMkZQWkhGaU9VbGxUVEZKTjBKVk5sRXliSFZrU3k5VFNVZzFiR1JaU2pFd1NqaFRiMlJ4TDBRNU1UUktjbFpwUkRoUGEwUnViRTVCU2xWRkwxZ3JWRXAzY0RKNlYybFBSWFo2YWl0NVdVdzRlVzVHV0VkUFdYUlFkV2R1UTBabmJXMHJibEZWT1RCdmNXTlVVSEVyWW1Rd1ZUbElXSFU1YVdweEwzWnRVSEJ4VWxKV1RrcHlVMG8wYUVWak1HZHBObHBzV2xWVVZXdFBXbEl4VFZKaGVEUkRMME54ZURnclVETTBZbEJ2Y1hoWk5tb3hWVUp6WlVsNlUyVTBNVEZQTWxsdE0zaHVjMlJrTWpoV2NESnVPRlJTY1UxWlpIQjNiSEI1YW1oTU9XSlJjbWhFYjB3dmFVRnJkbFZ3YzNScVJuRjVRMFZaT1M4NE5GbEZUeTlhTDNFMGVISXllVmsyYTBKaWJWVktabnA1TUZvdlVFOUlLekI2WWtSTVNpOHpZemRZVDFKSlYyMVBNM2RtYUVzNGNXSmxlRFJHYTJZeWNUVnlWRTFGWmpaRVdXWktOV1pXZG1NNU9VTm9VMFY2YjAwMmRYcExaMUV4VFdaMmNuSXdZVlJWU0ZBNFZVZ3paV3REVTJKQ2JVbHZlalEwUm1KRFJEUXJUVWt5YUV4TGNHeFFZWEJ0YzNKWWRIaDZTVVpKTlVKQ1dqSkJSM0V4WnpKSWRtTkdaVTRyY0VodldIVnVVbW92TUZVemFETXJlRzR5V1hsT2FVMHJXbkZZTmxKQk5XaDZVM0ZuZW5kRlpXMU5ZamhRZFV0WE9XUldSMUp4YVhKb1FVbHZPVXhOY20wNGVITk1PV1owWWtocFkxaFBaVU5UU1c0dldVaHdTRFJXVFRoTFoyeE5iazB3UjNkVVRVWlBha2xFVlc5R1dtZG5jV00wU0VOd1FXMDFSbkYwUldoWlYyRnpTeTl6VUcxNlEwOWhkMmd3UlZSdWJXUnpaMDluUjFKVU1rUmFNbTVxTWs1c1MzbENTMmN2SzA5S05XeEZabGwwYkRKS1ozb3ZVRmxJZEZvMlJXWjBkRkZhVm5OamVHeGxZamcxTkdoMFJTc3dTSFZKT0ZnM01FTk9iaXREZUdKNE9UbE5helEwWjNRd1RsbExPRFl2Y0ZadWNFcFJPV1ppT1dvMlZsbGFiamR0WVhKU1dUaHVaRzVCUjJ4Rk1IUkdjamRtT0hkRFprTktSbXBxWWtwTVZHeDRhbUZ6TmtoWFduSnpZWEJ6TmxSS1IwcDZTMmh4VFVkMWEzVnpaRXgyTW0wNVp6TlRUM0F6TWs5d1UxZERUbU0xY1UxeVVHSm9jRTVrVDAxak9VdFRSR0pxVlV4bFprNUZRWGRrTTBKbFQxbFNNemhSYTI5TGVuQjBWVFZ0VFZkUlIwWnRNa3BGU1Vob2FtdGxOV2d6Y25WUFVtTTVWMGN5TVhsRVVIRXJRa3BaY1dkQ2FVTjNRemRDYldwVVVGSXpOVlJRTVRNeFoxbE9aM1JLTTA5bGVFdFFRMWxFWkdwSFpHcHFXWGh5Y0dOT2RtUXpabnB0WnpoNVNXVm9XbVoxU2tzMGNHbHBOMjFpZDNVNWQwOUljSFZ1V0VGTVNtWnhNRFJQV0VJMmMyMTRUbmxNTm5kSFZWQjNUa1l4YUZsMU1XRjBhWEk0WkdsUFRYSlhhV042TWtoTFRscHpPUzlVZFZscWVsbDBjazFNZG5GaFFtbHFSRWwxYkZFMWJIWmlRVVl4UW5WQmRuTldWV1ZLTkd4cVJuQXhVM1lyUlhodE1rMXhXVEU1Y0hZemRWVkJOM2t2VTFka2IwVnhabGh1TmxCeU1uZ3pMM2htTkRORFp6QXdOa3BZU1hac1VuWkZSVzQyZWxoRmRuUm5TRVZqZURCT1RXc3dRamwwVVVWRlNXRTRPVVZDT0hvMVpVbzBUVkpJV0hnMldtOWhSSFptU2xoUFYzTlVURGgwVFd0eWMzUjBaUzgxYmpWWFEyOXRhMUZZVjNSNlJFTjJibGRZZVZGRWFHaElRblIwVUdoSE9VUTNRekJqV1ZOMGFHaEVXamd5TjFvMlNDOVRSMVJMVG5jemFVb3Jia0pXUVU1T1FUaFliMHB1VkVsTU5YcE1OMFJxT1ZOdFIyMU1kMk5LVkU1UFVHaFVVVkF5VERaNU9YVkVVVEJPUjAxVWRtRk9WVE5YTW1Kb00wTkhUMlV5T1Vsd2QxQm9lRk5xSzJGc1VXaEJhRFo2YVd0Q0szVlZZVVJVUldKSVpFRlBPVVZJUTIxTVdqUmhibkE0YjJGS2MxY3habWxuVG1GeFdtVkNVMGxTVkdaM01Ya3ZXblp2ZEUxb1ZsRjNXUzl3TUc5VVQwSjFWazR4ZERoYVZIQmxiREZLU0dkVVdqRkRUMWQwWWt0eFF6WnBWVEl3UTFFclRtRXJZMEZvVUdkWFFYVm5jak5rS3pkYUszRkpRWHBaVDI5ellVbDFSRzlMY0VGNU1IZGliSEZETDIxVU9UUnhNVzFLVDJsWmMwWkZSek5hWldWTlJVaG9RVFZMY0VKRU4yUk9jVVI0YldkSVZHWkxWVk5MYVZkaFdsYzFNbXcwUnpKblJITlpVSEJXZEROV05sSmpPR0l4V25aU1FYVmxSV1Y1ZUhSSVlVVmpUMk5aTTNwc2VtbDBiV2xuUzFkak5scDBNSFEyVXpCV1NuZHZOWGd6ZFhaeVZubzNabVZWV2xRNGRtSnFZekJTWjFoR1pEYzNNVGc1ZEVkUmIyeHpXRXBVWW1sYWJqVnFWa2R4TTBSUVpua3JRVVJXVlhSdVowdGtjM2RrZGtzNWQyTjNOV05ZV1UwcmExRlhURk5WYW1WYVNpOWliRTkyV1Zob2IxbFBkRU5XVjFWU2NFNXJhV0pHUjI1eFducDJPREl2VWxsTGVYTnNaMEpPVm1wNk5sTXllSGhDUms1akwzcHBUeXRNV2paMWQwNXpRMDV0Vld4VFNFSm5LekZIYkd0cE5VUmtSR3R5Y0RSM2QxQldWbGhpU0ZWVlZGcG1VemxFVEZBNE5XOUJkR05RY2xOQlRtb3lkVWRuTVdGSVpubExSVWh4YXpkVGRqTlRSbXBGY3pOS1ZtNWphbkZES3pGRVVtMHZSMlp3U1dNckszSkRka0p2U2tseFMxZEpVa0lyZUd4V1NEUTVlVVJzYTBvMmVIbFNjMmhhY0V0b05VTmthVGxhV1cxMEx6ZFVLMk5HV2xWR2RGaHpibGhsU1RRNVJHOUZMell4ZEZSaWVGaG9hRFZCY1U5SWJEaENLMlpVVDB3ekwxRlpkREZRTm1WR2FrWnlXRzlRUzI5aFZHVjFUMEV6V25CTVpuSjVNRVo2ZGtoT2FDdG9VV0pQUlhrMWJYa3JWa0p1UnpkcWJTODVUazFqVFRsMGNUTlpWV05aUzJkWWJtcGhOakIwVGpsaVEwSlpjbGhKY2tacE1EaGlTV2QzVDFkV1YyZEJSbU5QTTJOVFowa3habTF2VTNNMVUyUkRWSE5ETUdsQ1NVWmllRGRCYWpBNUswaEVVbU5EU0dNeFpub3ZUbEpYT0dwNU9IVlFWVFZNU2pjeWVrVjBSVEkxYVdJdlJFRTFNMjFFU201dlpGWXZjVFJwTjNGRlZrOUpUVXN2ZVVaTmJHaGxLMWxFYVZOdFlUTkZWVXBhUlRrNGVtdHlRbWcwTWxWV2JqY3piV0phVVdGQllUaFNNbTh3UzNKamRHNUNkMjlxYUdjck9HczVMMUJMYVZaRWFtMUJNamhYY1ZkNlZWazBSMjlaY2xGME9YWkhlSHBVV1VOdlprVTJZVVpOUTJ4VUwxUnNibVJWVFdwbFduWnZkMDFqTTA5bGQyeGtiRlp4YzFaRFRXMUxRMjFtUnpBMVFWQk5NWGRKT1hkWGNWVkdaVGhSYlhObk0wRjZiVTh5Y0hCYVRIZDJka0YzZUU5dlJqaFdSbUpxVlRaYVFqRXdabTgzYjBWbFVXbElaVTFRVFhWaVNWRlRVblp6SzA5UVF6VmlZVE5UV2xGdmJFUlpNblZGUjBabU0yZzVNRTFDVjJGSWVUQlZlRlpvUkV4UmFHeEZabXhTVGtwaVZIWkVURzFFTkdabmMwUkhOMWhEUkN0aVEwOXNXbk5XWlZaM1RtRm1SMWwxVm5SMGMwcDJTV1F2ZFRsM05HbEpTMEpJVWtOdWMxZHZVVmxQY0dwUmFXUXpVbEpzYzBWb1dpdFZSVVZhVFU5WU5EZHJURVl3VDNaa00zb3lhMVZYZWtOa1RTdHlaSGhGVFc4dlFsVjRNalJRWkhNNWJUVklNR2Q2Y1U4MlpYUXdkalozU1RoQ00xTnRRbGhNUTFCb1NqTk1iekE0T0RkUFlWSTFVVU5QY0d0b1EzWXJWRWxFTkdWUk9UZHdNak00SzFwNGJXVnRUMUJST1UxeVl6aGFUM2huV2pjMlNXbFlhRWcyWWpsSk5XOXJOMWsyUkZkNlQwcFRZMlpRV21oeWRtRm1XVFJrWkZOdlp5ODROa2x6ZDNSVVVHUlVXVkpRV1VWUVdtdGhNR3BEYlhKSGQzRnpSMHBRWmxaUVRIa3dUMFpRZERkMFpXSnZlVmQxTUVaUGFteEJXbVoyVFdVdmNucE5jM0J6TjJsSVNWcExja3huYzFVelFXZHliR3B1Tm0xdE1GVnZUV3AzZVRJclUyTnZXbXBEUzNKeVF6VllSM1ptWVVab1JsbEJVRGxOZFhaVFVrcHVOVlExU2s1NFdHVlVOWHBxYVhob1lsWTJkakkyY2tzeFRtaGtRa01yV1dWNVYyUk1kR2xYVW1SV1JuUlhNM2N3YkZGWlRVVnhjVkk0Y1RReFoxTlpTMjFyUVcxMGRYSjZVbVZUWjFGR2MwSjJiM1pqVTNaNE5YZGFSbXR4UXpBNVNGSkdiVGhuYUZCNU1EWnZhMVprTm05eWJHTnRjM0V4ZWtkWVEwOTBjMEZWYldWVlZqRm1OMmhIT0RBcmQyVTRURE0yZVc5cE5qSXpNV1Y0VDBkb1JrcFVVRGRyV1RaWU15OXdkMDV0ZFdobVQyOTBORzlGY0hkVllrSmtaR3A2ZDI5T1RHRXhMMEZ6YVU1SFNtUjBSRlZaVm5kUU5tSkpWV1ZQY2pCRVRXVlhibmR5VTFsblEweHBSbTVqZFdaVllUVllTRUYxSzFsR1Qxa3ZiMnBOVlZWUmRFSkpkbTFLYVhOMVUzcHJWRVpVZEVOVVdFUk9aVlF4UTNBNUwwRXZPRXBNUjFwVFFrVmFhMWhyZEhNd1dHOTNjbFV4VlRWd1pHSkdXa1kwUkRGSmNFOUNXR2t4VUN0eGFYQkVURTAxVTNOSlEwWmFVakJHYjI1RE5tbENXa0kzZUROQlJHZERVRlpDTm5KQk5uRXZMMUpSVUVGUVozVjZaMVl6UjFGaVZWb3ZUM1ZCUmt4VmVuSjNiRzkxZG14b0x5OWFNMVp2TWpkSE9IWTNlbkIxY2xSYWIyYzBVVVJCZDNwRVJITktOMGhKVEcxNmFHMVFOVE50YTFKS2VWbFZaMk55YkRkc2RHWmlWRzF2YlZBcmVHZHFRbUZLTVRrNFZIRm5iblV3UTBneVQxWm9TV0owV2xaNFR6QkdNVU5aYzBwa1YzVk9jbHBGTkcxMlNsbE9hRVZ4VUVJd2Mwc3lSbUY1VFdwU2VIWXJibkZtUzBSWk5XOXJSMll4YzNkSGRtZGpNWFozUTIxdmRtbzNRbnA2UkdoTVRubGtPRzVGTm5wWFdIY3pTRmh6U1dSSWVuSjFhbll4Y1hKRE1XbGhkbXRsU1RWTlpGRTJZWEI0VGt0RWRVNTBSVzVCVlhwRldsUkdkSG96V25GbWR6bDBhREJIZDBGRmJGRTRRVlZrU2sxTVluUTVjbWt5TUZneUsxWnRNVTlhZUM4dlZDdGFLMGc4TDBWdVkzSjVjSFJsWkVobFlXeDBhRU5sY25ScFptbGpZWFJsUGp4SlZqNHlOR2RzWkZOR1VFUkhTbWRYWlZaUFN6aDBlVk5CUFQwOEwwbFdQanhUYVdkdVlYUjFjbVUrUlVWeVVtTmlUekZyVlZkNWR6Wm9aVTFUWmpaS1MyUXhXR2xsYzNBck0ybGxNMFEyY25CVGNVeEdNRDA4TDFOcFoyNWhkSFZ5WlQ0OEwwOXdZWEYxWlVobFlXeDBhRU5sY25ScFptbGpZWFJsUGc9PTwvSGVhbHRoQ2VydGlmaWNhdGVCbG9iPjwvSGVhbHRoQ2VydGlmaWNhdGVWYWxpZGF0aW9uUmVxdWVzdD4=";

            TestContext.WriteLine($"CorrelationID={correlationId}");

            Func<Task> asyncMethod = async () =>
            {
                // Get device client
                var connectionString = await GetTestDeviceConnectionString();
                var deviceClient = DeviceClient.CreateFromConnectionString(connectionString, Microsoft.Azure.Devices.Client.TransportType.Mqtt);

                // Set up Nonce
                await NonceCloudTable.Instance.SetNonceForDeviceAsync(TestDeviceId, ValidNonce);

                // Set up device twin
                var desiredProperty = new DeviceHealthAttestationDataContract.DesiredProperty();
                desiredProperty.Endpoint = string.Empty;
                await SetTestDeviceTwin(desiredProperty);

                // Send Health report
                var properties = new Dictionary<string, string>();
                properties.Add("MessageType", DeviceHealthAttestationDataContract.HealthReportTag);
                var msg = new DeviceHealthAttestationDataContract.HealthReport()
                {
                    CorrelationId = correlationId,
                    HealthCertificate = ValidHealthCertificate,
                };
                await deviceClient.SendMessageAsync(JsonConvert.SerializeObject(msg), properties);

                // Write for report to get processed
                Thread.Sleep(TimeSpan.FromSeconds(10));

                // Ensure a new report is written with that correlationID
                var reports = ReportTable.Instance.Query(DateTimeOffset.Now - TimeSpan.FromMinutes(1), DateTimeOffset.Now, TestDeviceId, correlationId);
                Assert.AreEqual(1, reports.Count());
            };

            asyncMethod().Wait();
        }

        private async Task<string> GetTestDeviceConnectionString()
        {

            var registryManager = RegistryManager.CreateFromConnectionString(IotHubOwnerConnectionString);

            // Create Test device if not exist
            var device = await registryManager.GetDeviceAsync(TestDeviceId);
            if (device == null)
            {
                device = new Device(TestDeviceId);
                device = await registryManager.AddDeviceAsync(device);
            }

            // Return DeviceClient
            var iotHubName = "alanch-test-iothub";
            var connectionString = $"HostName={iotHubName}.azure-devices.net;DeviceId={device.Id};SharedAccessKey={device.Authentication.SymmetricKey.PrimaryKey}";
            return connectionString;
        }

        private async Task SetTestDeviceTwin(DeviceHealthAttestationDataContract.DesiredProperty desired)
        {
            var registryManager = RegistryManager.CreateFromConnectionString(IotHubOwnerConnectionString);

            string prefix = "{ \"properties\" : {\"desired\":{\"microsoft\":{\"management\":{\"" + DeviceHealthAttestationDataContract.JsonSectionName + "\":";
            string suffix = "}}}}}";
            var json = JsonConvert.SerializeObject(desired);

            var twin = await registryManager.GetTwinAsync(TestDeviceId);
            var twinPatch = JsonConvert.DeserializeObject<Twin>(prefix + json + suffix);
            twinPatch.DeviceId = TestDeviceId;
            twinPatch.ETag = twin.ETag;
            await registryManager.UpdateTwinAsync(TestDeviceId, twinPatch, twin.ETag);
        }
    }
}
