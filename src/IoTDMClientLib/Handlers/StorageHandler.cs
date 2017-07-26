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

using DMDataContract;
using Microsoft.Devices.Management.Message;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Text;
using System.Threading.Tasks;
using Windows.Foundation.Diagnostics;
using Windows.Storage;

namespace Microsoft.Devices.Management
{
    class StorageHandler : IClientDirectMethodHandler
    {
        const int SuccessCode = 0;
        const string JsonNamesList = "list";
        const string JsonErrorCode = "errorCode";
        const string JsonErrorMessage = "errorMessage";
        const string JsonFolder = "folder";
        const string JsonFile = "file";
        const string JsonConnectionString = "connectionString";
        const string JsonContainer = "container";

        const string MethodEnumDMFolders = DMJSonConstants.DTWindowsIoTNameSpace + ".enumDMFolders";
        const string MethodEnumDMFiles = DMJSonConstants.DTWindowsIoTNameSpace + ".enumDMFiles";
        const string MethodDeleteDMFile = DMJSonConstants.DTWindowsIoTNameSpace + ".deleteDMFile";
        const string MethodUploadDMFile = DMJSonConstants.DTWindowsIoTNameSpace + ".uploadDMFile";

        public StorageHandler(IClientHandlerCallBack callback, ISystemConfiguratorProxy systemConfiguratorProxy)
        {
            this._systemConfiguratorProxy = systemConfiguratorProxy;
            this._callback = callback;
        }

        // IClientDirectMethodHandler
        public IReadOnlyDictionary<string, Func<string, Task<string>>> GetDirectMethodHandler()
        {
            return new Dictionary<string, Func<string, Task<string>>>()
                {
                    { MethodEnumDMFolders , EnumDMFolders },
                    { MethodEnumDMFiles , EnumDMFiles },
                    { MethodDeleteDMFile , DeleteDMFile },
                    { MethodUploadDMFile , UploadDMFile },
                };
        }

        private string GetParameter(JObject jsonParamsObject, string name, int errorCode, string errorMessage)
        {
            string value = "";
            if (!Utils.TryGetString(jsonParamsObject, name, out value))
            {
                throw new Error(errorCode, errorMessage);
            }
            Logger.Log(name + " :" + value, LoggingLevel.Verbose);
            return value;
        }

        private string BuildListJsonString(StringListResponse responseList)
        {
            // JsonConvert.SerializeObject does not work with responseList.List.
            StringBuilder jsonArrayString = new StringBuilder();
            foreach (string s in responseList.List)
            {
                Logger.Log("Found: " + s, LoggingLevel.Verbose);
                if (jsonArrayString.Length > 0)
                {
                    jsonArrayString.Append(",\n");
                }
                jsonArrayString.Append("\"" + s + "\"");
            }

            StringBuilder sb = new StringBuilder();
            sb.Append("    \"" + JsonNamesList + "\": [\n");
            if (jsonArrayString.Length > 0)
            {
                sb.Append(jsonArrayString.ToString());
                sb.Append("\n");
            }
            sb.Append("    ]\n");

            Logger.Log("Json List: " + sb.ToString(), LoggingLevel.Verbose);

            return sb.ToString();
        }

        private Task<string> BuildMethodJsonResponse(string output, int errorCode, string errorMessage)
        {
            StringBuilder sb = new StringBuilder();
            sb.Append("{\n");
            if (output.Length > 0)
            {
                sb.Append(output);
                sb.Append(",");
            }
            sb.Append("\"" + JsonErrorCode + "\" : "+ errorCode + ",");
            sb.Append("\"" + JsonErrorMessage + "\" : \"" + errorMessage + "\",");
            sb.Append("}\n");

            string methodResponse = sb.ToString();

            Logger.Log("Method Response: " + methodResponse, LoggingLevel.Verbose);
            return Task.FromResult(methodResponse);
        }

        private Task<string> EnumDMFolders(string jsonParamString)
        {
            Logger.Log("Enumerating DM folders...", LoggingLevel.Information);

            try
            {
                var request = new GetDMFoldersRequest();
                Task<IResponse> response = _systemConfiguratorProxy.SendCommand(request);
                string responseListString = BuildListJsonString(response.Result as StringListResponse);
                return BuildMethodJsonResponse(responseListString, 0, "");
            }
            catch(Exception err)
            {
                return BuildMethodJsonResponse("", err.HResult, err.Message);
            }
        }

        private Task<string> EnumDMFiles(string jsonParamString)
        {
            Logger.Log("Enumerating DM files...", LoggingLevel.Information);

            try
            {
                object paramsObject = JsonConvert.DeserializeObject(jsonParamString);
                if (paramsObject == null || !(paramsObject is JObject))
                {
                    throw new Error(ErrorCodes.INVALID_PARAMS, "Invalid enumDMFiles parameters.");
                }

                JObject jsonParamsObject = (JObject)paramsObject;

                var request = new GetDMFilesRequest();
                request.DMFolderName = GetParameter(jsonParamsObject, JsonFolder, ErrorCodes.INVALID_FOLDER_PARAM, "Invalid or missing folder parameter.");
                Task<IResponse> response = _systemConfiguratorProxy.SendCommand(request);
                string responseListString = BuildListJsonString(response.Result as StringListResponse);
                return BuildMethodJsonResponse(responseListString, 0, "");
            }
            catch (Exception err)
            {
                return BuildMethodJsonResponse("", err.HResult, err.Message);
            }
        }

        private Task<string> DeleteDMFile(string jsonParamString)
        {
            Logger.Log("Deleting DM file...", LoggingLevel.Information);

            try
            {
                object paramsObject = JsonConvert.DeserializeObject(jsonParamString);
                if (paramsObject == null || !(paramsObject is JObject))
                {
                    throw new Error(ErrorCodes.INVALID_PARAMS, "Invalid enumDMFiles parameters.");
                }

                JObject jsonParamsObject = (JObject)paramsObject;

                var request = new Message.DeleteDMFileRequest();
                request.DMFolderName = GetParameter(jsonParamsObject, JsonFolder, ErrorCodes.INVALID_FOLDER_PARAM, "Invalid or missing folder parameter.");
                request.DMFileName = GetParameter(jsonParamsObject, JsonFile, ErrorCodes.INVALID_FILE_PARAM, "Invalid or missing folder parameter.");
                Task<IResponse> response = _systemConfiguratorProxy.SendCommand(request);

                StringResponse responseList = response.Result as StringResponse;

                return BuildMethodJsonResponse("", (int)responseList.Status, responseList.Response);
            }
            catch (Exception err)
            {
                return BuildMethodJsonResponse("", err.HResult, err.Message);
            }
        }

        private async Task UploadDMFileAsync(string jsonParamString)
        {
            Logger.Log("Uploading DM file...", LoggingLevel.Information);

            try
            {
                object paramsObject = JsonConvert.DeserializeObject(jsonParamString);
                if (paramsObject == null || !(paramsObject is JObject))
                {
                    throw new Error(ErrorCodes.INVALID_PARAMS, "Invalid enumDMFiles parameters.");
                }

                JObject jsonParamsObject = (JObject)paramsObject;

                string folderName = GetParameter(jsonParamsObject, JsonFolder, ErrorCodes.INVALID_FOLDER_PARAM, "Invalid or missing folder parameter.");
                string fileName = GetParameter(jsonParamsObject, JsonFile, ErrorCodes.INVALID_FILE_PARAM, "Invalid or missing folder parameter.");

                var info = new AzureFileTransferInfo();
                info.ConnectionString = GetParameter(jsonParamsObject, JsonConnectionString, ErrorCodes.INVALID_CONNECTION_STRING_PARAM, "Invalid or missing connection string parameter.");
                info.ContainerName = GetParameter(jsonParamsObject, JsonContainer, ErrorCodes.INVALID_CONTAINER_PARAM, "Invalid or missing container parameter.");
                info.BlobName = fileName;
                info.Upload = true;
                info.LocalPath = Constants.IoTDMFolder + "\\" + folderName + "\\" + fileName;
                info.AppLocalDataPath = ApplicationData.Current.TemporaryFolder.Path + "\\" + fileName;

                AzureFileTransferRequest request = new AzureFileTransferRequest(info);
                var response = _systemConfiguratorProxy.SendCommand(request);
                if (response.Result.Status == ResponseStatus.Success)
                {
                    Logger.Log("File copied to UWP application temporary folder...", LoggingLevel.Information);
                    var appLocalDataFile = await ApplicationData.Current.TemporaryFolder.GetFileAsync(fileName);
                    Logger.Log("Uploading file...", LoggingLevel.Information);
                    await IoTDMClient.AzureBlobFileTransfer.UploadFile(info, appLocalDataFile);
                    Logger.Log("Upload done. Deleting local temporary file...", LoggingLevel.Information);
                    await appLocalDataFile.DeleteAsync();
                    Logger.Log("Temporary file deleted..", LoggingLevel.Information);
                }
            }
            catch (Exception err)
            {
                BuildMethodJsonResponse("", err.HResult, err.Message);
            }
        }

        private Task<string> UploadDMFile(string jsonParam)
        {
            UploadDMFileAsync(jsonParam);

            return BuildMethodJsonResponse("" /*payload*/, SuccessCode, "" /*error message*/);
        }

        private ISystemConfiguratorProxy _systemConfiguratorProxy;
        private IClientHandlerCallBack _callback;
    }
}
