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

#include <stdafx.h>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include "..\SharedUtilities\Logger.h"
#include "..\SharedUtilities\Utils.h"
#include "MdmProvision.h"
#include "DiagnosticLogCSP.h"

using namespace std;
using namespace Microsoft::Devices::Management::Message;

const wchar_t* DiagnosticLogCSPWorkingFolder = L"C:\\Data\\Users\\Public\\Documents";

const wchar_t* CSPQuerySuffix = L"?list=StructData";
const wchar_t* CSPCollectorsRoot = L"./Vendor/MSFT/DiagnosticLog/EtwLog/Collectors";
const wchar_t* CSPTrue = L"TRUE";
const wchar_t* CSPNodeType = L"node";

// Collectors
const wchar_t* CSPTraceStatus = L"TraceStatus";
const wchar_t* CSPTraceLogFileMode = L"TraceLogFileMode";
const wchar_t* CSPLogFileSizeLimitMB = L"LogFileSizeLimitMB";
const wchar_t* CSPProvidersNode = L"Providers";
const wchar_t* CSPTraceControl = L"TraceControl";
const wchar_t* CSPTraceControlStart = L"START";
const wchar_t* CSPTraceControlStop = L"STOP";

// Providers
const wchar_t* CSPTraceLevel = L"TraceLevel";
const wchar_t* CSPKeywords = L"Keywords";
const wchar_t* CSPState = L"State";

// Download Channel
const wchar_t* CSPDataChannel = L"./Vendor/MSFT/DiagnosticLog/FileDownload/DMChannel";
const wchar_t* CSPBlockCount = L"BlockCount";
const wchar_t* CSPBlockIndexToRead = L"BlockIndexToRead";
const wchar_t* CSPBlockData = L"BlockData";

const wchar_t* JsonNoString = L"no";
const wchar_t* JsonYesString = L"yes";
const wchar_t* JsonFileModeSequential = L"sequential";
const wchar_t* JsonFileModeCircular = L"circular";
const wchar_t* JsonTraceLevelCritical = L"critical";
const wchar_t* JsonTraceLevelError = L"error";
const wchar_t* JsonTraceLevelWarning = L"warning";
const wchar_t* JsonTraceLevelInformation = L"information";
const wchar_t* JsonTraceLevelVerbose = L"verbose";



IResponse^ DiagnosticLogCSP::HandleGetEventTracingConfiguration(IRequest^ request)
{
    TRACE(__FUNCTION__);

    map<wstring, CollectorReportedConfiguration^> encounteredCollectorsMap;

    GetEventTracingConfigurationResponse^ response = ref new GetEventTracingConfigurationResponse(ResponseStatus::Success);

    std::function<void(std::vector<std::wstring>&, std::wstring&)> valueHandler =
        [response, &encounteredCollectorsMap](vector<wstring>& uriTokens, wstring& value)
    {
        if (uriTokens.size() < 7)
        {
            return;
        }

        wstring cspCollectorName = uriTokens[6];
        CollectorReportedConfiguration^ currentCollector;

        // 0/__1___/_2__/______3______/__4___/____5_____/___6___/
        // ./Vendor/MSFT/DiagnosticLog/EtwLog/Collectors/AzureDM
        map<wstring, CollectorReportedConfiguration^>::iterator it = encounteredCollectorsMap.find(cspCollectorName);
        if (it == encounteredCollectorsMap.end())
        {
            wstring collectorRegistryPath = RegEventTracing;
            collectorRegistryPath += L"\\";
            collectorRegistryPath += cspCollectorName.c_str();

            currentCollector = ref new CollectorReportedConfiguration();
            currentCollector->Name = ref new String(cspCollectorName.c_str());

            wstring reportToDeviceTwin = Utils::ReadRegistryValue(collectorRegistryPath, RegReportToDeviceTwin, JsonNoString /*default*/);
            currentCollector->ReportToDeviceTwin = ref new String(reportToDeviceTwin.c_str());
            currentCollector->CSPConfiguration->LogFileFolder = ref new String(Utils::ReadRegistryValue(collectorRegistryPath, RegEventTracingLogFileFolder, cspCollectorName /*default*/).c_str());
            currentCollector->CSPConfiguration->LogFileName = ref new String(Utils::ReadRegistryValue(collectorRegistryPath, RegEventTracingLogFileName, L"" /*default*/).c_str());

            // Add it to the collectors list...
            response->Collectors->Append(currentCollector);

            // Save it in the collectors map so that we can find it easily next time 
            // we need it while processing another token...
            encounteredCollectorsMap[cspCollectorName] = currentCollector;
        }
        else
        {
            currentCollector = it->second;
        }

        if (uriTokens.size() >= 8)
        {
            // 0/__1___/_2__/______3______/__4___/____5_____/___6___/____7______/
            // ./Vendor/MSFT/DiagnosticLog/EtwLog/Collectors/AzureDM/TraceStatus

            if (uriTokens[7] == CSPTraceStatus)
            {
                currentCollector->CSPConfiguration->Started = std::stoi(value) == 1 ? true : false;
            }
            else if (uriTokens[7] == CSPTraceLogFileMode)
            {
                currentCollector->CSPConfiguration->TraceLogFileMode = ref new String(std::stoi(value) == 1 ? JsonFileModeSequential : JsonFileModeCircular);
            }
            else if (uriTokens[7] == CSPLogFileSizeLimitMB)
            {
                currentCollector->CSPConfiguration->LogFileSizeLimitMB = std::stoi(value);
            }
        }

        // Is this something under the Providers node?
        if (uriTokens.size() >= 9 && uriTokens[7] == CSPProvidersNode)
        {
            // 0/__1___/_2__/______3______/__4___/____5_____/___6___/____7____/_8__
            // ./Vendor/MSFT/DiagnosticLog/EtwLog/Collectors/AzureDM/Providers/guid

            // Make sure the Providers list exists...
            if (currentCollector->CSPConfiguration->Providers == nullptr)
            {
                currentCollector->CSPConfiguration->Providers = ref new Vector<ProviderConfiguration^>();
            }

            // Do we already have this provider?
            ProviderConfiguration^ currentProvider;
            for (auto provider : currentCollector->CSPConfiguration->Providers)
            {
                if (0 == _wcsicmp(provider->Guid->Data(), uriTokens[8].c_str()))
                {
                    currentProvider = provider;
                }
            }

            // If we don't already have the provider, create a new one...
            if (currentProvider == nullptr)
            {
                ProviderConfiguration^ provider = ref new ProviderConfiguration();
                provider->Guid = ref new String(uriTokens[8].c_str());
                currentCollector->CSPConfiguration->Providers->Append(provider);

                currentProvider = provider;
            }

            // Is this a sub-property of the current provider?
            if (uriTokens.size() >= 10)
            {
                // 0/__1___/_2__/______3______/__4___/____5_____/___6___/___7_____/_8__/____9____
                // ./Vendor/MSFT/DiagnosticLog/EtwLog/Collectors/AzureDM/Providers/guid/TraceLevel

                if (uriTokens[9] == CSPTraceLevel)
                {
                    wstring jsonValue;
                    if (value == L"1")
                    {
                        jsonValue = JsonTraceLevelCritical;
                    }
                    else if (value == L"2")
                    {
                        jsonValue = JsonTraceLevelError;
                    }
                    else if (value == L"3")
                    {
                        jsonValue = JsonTraceLevelWarning;
                    }
                    else if (value == L"4")
                    {
                        jsonValue = JsonTraceLevelInformation;
                    }
                    else if (value == L"5")
                    {
                        jsonValue = JsonTraceLevelVerbose;
                    }
                    currentProvider->TraceLevel = ref new String(jsonValue.c_str());
                }
                else if (uriTokens[9] == CSPKeywords)
                {
                    currentProvider->Keywords = ref new String(value.c_str());
                }
                else if (uriTokens[9] == CSPState)
                {
                    currentProvider->Enabled = (0 == _wcsicmp(value.c_str(), CSPTrue)) ? true : false;
                }
            }
        }
    };

    wstring cspQuery;
    cspQuery += CSPCollectorsRoot;
    cspQuery += CSPQuerySuffix;

    MdmProvision::RunGetStructData(cspQuery, valueHandler);

    return response;
}

wstring DiagnosticLogCSP::GetFormattedTime()
{
    TRACE(__FUNCTION__);

    time_t now;
    time(&now);

    tm nowParsed;
    errno_t errCode = localtime_s(&nowParsed, &now);
    if (errCode != 0)
    {
        string errorMessage = "Error: could not obtain local time.";
        TRACE(errorMessage.c_str());
        throw DMException(errorMessage.c_str());
    }

    basic_ostringstream<wchar_t> nowString;
    nowString << (nowParsed.tm_year + 1900) << L"_";
    nowString << setw(2) << setfill(L'0') << (nowParsed.tm_mon + 1) << L"_";
    nowString << setw(2) << setfill(L'0') << nowParsed.tm_mday << L"_";
    nowString << setw(2) << setfill(L'0') << nowParsed.tm_hour << L"_";
    nowString << setw(2) << setfill(L'0') << nowParsed.tm_min << L"_";
    nowString << setw(2) << setfill(L'0') << nowParsed.tm_sec;
    return nowString.str();
}

void DiagnosticLogCSP::CreateEtlFile(CollectorDesiredConfiguration^ collector)
{
    TRACE(__FUNCTION__);

    // The etl file will be created in a folder like this:
    // c:\Data\Users\DefaultAccount\AppData\Local\Temp\<collector's folder>

    // Construct the data channel...
    wstring collectorFileCSPPath;
    collectorFileCSPPath += CSPDataChannel;
    collectorFileCSPPath += L"/";
    collectorFileCSPPath += collector->Name->Data();

    // Read data from CSP into our buffers...
    vector<vector<char>> decryptedEtlBuffer;
    int blockCount = 0;
    if (MdmProvision::TryGetNumber(collectorFileCSPPath + L"/" + CSPBlockCount, blockCount))
    {
        for (int i = 0; i < blockCount; ++i)
        {
            MdmProvision::RunSet(collectorFileCSPPath + L"/" + CSPBlockIndexToRead, i);
            wstring blockData = MdmProvision::RunGetBase64(collectorFileCSPPath + L"/BlockData");

            vector<char> decryptedBlock;
            Utils::Base64ToBinary(blockData, decryptedBlock);
            decryptedEtlBuffer.push_back(decryptedBlock);
        }
    }

    // Make sure the target folder exists...
    wstring etlFolderName;
    etlFolderName += Utils::GetDmUserFolder();
    etlFolderName += L"\\";
    etlFolderName += collector->CSPConfiguration->LogFileFolder->Data();
    CreateDirectory(etlFolderName.c_str(), NULL);

    // Construct the file name...
    wstring etlFileName;
    if (collector->CSPConfiguration->LogFileName->Length() == 0)
    {
        etlFileName += collector->Name->Data();
        etlFileName += L"_";
        etlFileName += GetFormattedTime();
        etlFileName += L".etl";
    }
    else
    {
        etlFileName = collector->CSPConfiguration->LogFileName->Data();
    }

    wstring etlFullFileName = etlFolderName + L"\\" + etlFileName;
    TRACEP(L"ETL Full File Name: ", etlFullFileName.c_str());

    // Write the buffers to disk...
    ofstream etlFile(etlFullFileName, ios::out | ios::binary);
    for (auto it = decryptedEtlBuffer.begin(); it != decryptedEtlBuffer.end(); it++)
    {
        etlFile.write(it->data(), it->size());
    }
    etlFile.close();
}

void DiagnosticLogCSP::ApplyCollectorConfiguration(const wstring& cspRoot, CollectorDesiredConfiguration^ collector)
{
    TRACE(__FUNCTION__);

    // Some validation...

    // If there is no configuration, there is no need to continue.
    if (collector->CSPConfiguration == nullptr)
    {
        TRACE(L"Warning: CSPConfiguration is nullptr.");
        return;
    }

    // ETL files cannot be saved to the IoTDM folder.
    // They have to be saved to a subfolder of IoTDM.
    if (collector->CSPConfiguration->LogFileFolder == nullptr ||
        collector->CSPConfiguration->LogFileFolder->Length() == 0)
    {
        TRACE(L"Warning: LogFileFolder is null or empty. Using collector name as the LogFileFolder.");
        collector->CSPConfiguration->LogFileFolder = collector->Name;
    }

    // Make sure the user is not trying to access any files outside the IoTDM folder.
    if (nullptr != wcsstr(collector->CSPConfiguration->LogFileFolder->Data(), L"..") ||
        nullptr != wcsstr(collector->CSPConfiguration->LogFileFolder->Data(), L"\\") ||
        nullptr != wcsstr(collector->CSPConfiguration->LogFileFolder->Data(), L"/"))
    {
        string errorMessage = "Error: LogFileFolder cannot contain '/', '\\', or '..'.";
        TRACE(errorMessage.c_str());
        throw DMException(errorMessage.c_str());
    }

    if (collector->CSPConfiguration->LogFileName->Length() != 0)
    {
        if (nullptr != wcsstr(collector->CSPConfiguration->LogFileName->Data(), L"..") ||
            nullptr != wcsstr(collector->CSPConfiguration->LogFileName->Data(), L"\\") ||
            nullptr != wcsstr(collector->CSPConfiguration->LogFileName->Data(), L"/"))
        {
            string errorMessage = "Error: LogFileName cannot contain '/', '\\', or '..'.";
            TRACE(errorMessage.c_str());
            throw DMException(errorMessage.c_str());
        }
    }

    // Build paths...
    const wstring collectorCSPPath = cspRoot + L"/" + collector->Name->Data();
    const wstring providersCSPPath = collectorCSPPath + L"/" + CSPProvidersNode;

    wstring collectorRegistryPath = RegEventTracing;
    collectorRegistryPath += L"\\";
    collectorRegistryPath += collector->Name->Data();

    // Add CSP node and set its properties...
    MdmProvision::RunAdd(cspRoot, collector->Name->Data());
    Utils::WriteRegistryValue(collectorRegistryPath, RegReportToDeviceTwin, collector->ReportToDeviceTwin->Data());
    Utils::WriteRegistryValue(collectorRegistryPath, RegEventTracingLogFileFolder, collector->CSPConfiguration->LogFileFolder->Data());
    Utils::WriteRegistryValue(collectorRegistryPath, RegEventTracingLogFileName, collector->CSPConfiguration->LogFileName->Data());
    MdmProvision::RunSet(collectorCSPPath + L"/LogFileSizeLimitMB", collector->CSPConfiguration->LogFileSizeLimitMB);
    MdmProvision::RunSet(collectorCSPPath + L"/TraceLogFileMode", collector->CSPConfiguration->TraceLogFileMode == L"sequential" ? 1 : 2);

    // Capture which providers are already part of this CSP collector configuration...
    wstring providersString = MdmProvision::RunGetString(providersCSPPath, true /*optional*/);

    // Iterate though each desired provider and add/apply its settings...
    for each (ProviderConfiguration^ provider in collector->CSPConfiguration->Providers)
    {
        wstring providerCSPPath = collectorCSPPath + L"/" + CSPProvidersNode + L"/" + provider->Guid->Data();

        // Is the provider already part of this CSP collector configuration?
        if (wstring::npos == providersString.find(provider->Guid->Data()))
        {
            MdmProvision::RunAddTyped(providerCSPPath, CSPNodeType);
        }

        int traceLevel = 0;
        if (0 == _wcsicmp(provider->TraceLevel->Data(), JsonTraceLevelCritical))
        {
            traceLevel = 1;
        }
        else if (0 == _wcsicmp(provider->TraceLevel->Data(), JsonTraceLevelError))
        {
            traceLevel = 2;
        }
        else if (0 == _wcsicmp(provider->TraceLevel->Data(), JsonTraceLevelWarning))
        {
            traceLevel = 3;
        }
        else if (0 == _wcsicmp(provider->TraceLevel->Data(), JsonTraceLevelInformation))
        {
            traceLevel = 4;
        }
        else if (0 == _wcsicmp(provider->TraceLevel->Data(), JsonTraceLevelVerbose))
        {
            traceLevel = 5;
        }

        MdmProvision::RunSet(providerCSPPath + L"/" + CSPState, provider->Enabled);
        MdmProvision::RunSet(providerCSPPath + L"/" + CSPKeywords, wstring(provider->Keywords->Data()));
        MdmProvision::RunSet(providerCSPPath + L"/" + CSPTraceLevel, traceLevel);
    }

    // Finally process the started/stopped status...
    unsigned int traceStatus = MdmProvision::RunGetUInt(collectorCSPPath + L"/" + CSPTraceStatus);
    if (collector->CSPConfiguration->Started)
    {
        if (traceStatus == 0 /*stopped*/)
        {
            TRACEP(L"Starting logging for ", collector->Name->Data());
            MdmProvision::RunExecWithParameters(collectorCSPPath + L"/" + CSPTraceControl, CSPTraceControlStart);
        }
    }
    else
    {
        if (traceStatus == 1 /*started*/)
        {
            TRACEP(L"Stopping logging for ", collector->Name->Data());
            MdmProvision::RunExecWithParameters(collectorCSPPath + L"/" + CSPTraceControl, CSPTraceControlStop);

            CreateEtlFile(collector);
        }
    }
}

IResponse^ DiagnosticLogCSP::HandleSetEventTracingConfiguration(IRequest^ request)
{
    TRACE(__FUNCTION__);

    // ToDo: There is a bug in the CSP where if C:\Data\Users\Public\Documents
    //       does not exist, it fails to start capturing events.
    //       To work around that, we are making sure the documents folder exists.
    Utils::EnsureFolderExists(DiagnosticLogCSPWorkingFolder);

    auto eventTracingConfiguration = dynamic_cast<SetEventTracingConfigurationRequest^>(request);
    for each (CollectorDesiredConfiguration^ collector in eventTracingConfiguration->Collectors)
    {
        ApplyCollectorConfiguration(CSPCollectorsRoot, collector);
    }

    return ref new StringResponse(ResponseStatus::Success, ref new String(), DMMessageKind::SetEventTracingConfiguration);
}