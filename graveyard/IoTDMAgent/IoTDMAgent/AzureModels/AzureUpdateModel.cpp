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
#include "stdafx.h"
#include <filesystem>
#include "AzureUpdateModel.h"
#include "..\Utilities\AzureStorage.h"
#include "..\LocalMachine\UpdateEngine.h"

using namespace Windows::Data::Json;
using namespace Windows::Foundation::Collections;
using namespace Platform;
using namespace std;
using namespace tr2::sys;

#define UpdateDownloaded L"downloaded"
#define UpdateInstalled L"installed"
#define UpdateState L"state"

AzureUpdateModel::AzureUpdateModel(const wstring& updatesRootFolder, const wstring& manifestFileName, const UpdateEngine& updateEngine) :
    _manifestDownloaded(false),
    _cabsDownloaded(false),
    _cabsInstalled(false)
{
    TRACE(L"AzureUpdateModel::AzureUpdateModel()");

    _updatesRootFolder = updatesRootFolder;

    // set manifest file name
    _manifestFileName = manifestFileName;
    _manifestFullFileName = _updatesRootFolder + L"\\" AzureManifestsFolder + L"\\" + _manifestFileName;
    _manifestDownloaded = Utils::FileExists(_manifestFullFileName);

    // set id
    _id = manifestFileName;

    // read cab data
    if (_manifestDownloaded)
    {
        ParseManifest(_updatesRootFolder, _manifestFullFileName, _cabsData /*out*/);

        // update the downloaded state
        _cabsDownloaded = VerifyCabsDownloaded(_updatesRootFolder, _cabsData);

        // update the installed state
        _cabsInstalled = VerifyCabsInstalled(_cabsData, updateEngine);
    }
}

void AzureUpdateModel::ParseManifest(const wstring& updatesLocalRoot, const wstring& manifestFileName, vector<CabData>& cabsData)
{
    TRACE(L"AzureUpdateModel::ParseUpdateManifest()");

    cabsData.clear();

    wifstream manifestInputStream(manifestFileName);
    if (!manifestInputStream)
    {
        throw DMException("Unable to open input stream for manifest file");
    }

    wstring currentManifestLine;
    while (getline(manifestInputStream, currentManifestLine))
    {
        if (currentManifestLine.empty())
        {
            continue;
        }

        currentManifestLine = Utils::TrimString<wstring>(currentManifestLine, L" \t\r\n");

        vector<wstring> tokens;
        Utils::SplitString(currentManifestLine, L',', tokens);
        assert(tokens.size() == 4);

        CabData cabData;
        cabData.containerName = tokens[0];
        cabData.cabFullFileName = updatesLocalRoot + L"\\" AzurePayloadFolder L"\\" + cabData.containerName + L"\\" + tokens[1];
        cabData.id = tokens[2];
        cabData.version = tokens[3];

        TRACEP(L"container = ", cabData.containerName.c_str());
        TRACEP(L"cab name  = ", cabData.cabFullFileName.c_str());
        TRACEP(L"id        = ", cabData.id.c_str());
        TRACEP(L"version   = ", cabData.version.c_str());

        cabsData.push_back(cabData);
    }
    manifestInputStream.close();
}

bool AzureUpdateModel::VerifyCabsDownloaded(const wstring& updatesLocalRoot, const vector<CabData> cabsData)
{
    TRACE(L"AzureUpdateModel::VerifyDownloaded()");

    bool downloaded = true;
    for (const CabData& cabData : cabsData)
    {
        downloaded = Utils::FileExists(cabData.cabFullFileName);
        if (!downloaded)
        {
            break;
        }
        // ToDo: we should verify the cab identity and version too.
    }

    if (downloaded)
    {
        TRACE(L"All cabs are present locally.");
    }
    else
    {
        TRACE(L"Not all cabs are present locally.");
    }

    return downloaded;
}

bool AzureUpdateModel::VerifyCabsInstalled(const vector<CabData>& cabsData, const UpdateEngine& updateEngine)
{
    TRACE(L"AzureUpdateModel::VerifyInstalled()");

    bool installed = true;
    for (const CabData& cabData : cabsData)
    {
        installed = updateEngine.IsInstalled(cabData.id, cabData.version);
        if (!installed)
        {
            break;
        }
    }

    if (installed)
    {
        TRACE(L"All cabs are installed.");
    }
    else
    {
        TRACE(L"Not all cabs are installed.");
    }

    return installed;
}

bool AzureUpdateModel::IsDownloaded() const
{
    return _manifestDownloaded && _cabsDownloaded;
}

bool AzureUpdateModel::IsInstalled() const
{
    return _cabsInstalled;
}

void AzureUpdateModel::Download(const wstring& connectionString)
{
    TRACE(L"AzureUpdateModel::Download()");

    if (!_manifestDownloaded)
    {
        path fullPath(_manifestFullFileName);
        AzureStorage::Download(connectionString, AzureManifestsFolder, _manifestFileName, fullPath.parent_path());
        _manifestDownloaded = true;
        ParseManifest(_updatesRootFolder, _manifestFullFileName, _cabsData /*out*/);
    }

    for (const CabData& cabData : _cabsData)
    {
        path fullPath(cabData.cabFullFileName);
        AzureStorage::Download(connectionString, cabData.containerName, fullPath.filename(), fullPath.parent_path());
    }

    _cabsDownloaded = VerifyCabsDownloaded(_updatesRootFolder, _cabsData);
}

void AzureUpdateModel::Install()
{
    TRACE(L"AzureUpdateModel::Install()");

    for (const CabData& cabData : _cabsData)
    {
        UpdateEngine::Stage(cabData.cabFullFileName);
    }
    UpdateEngine::CommitStaged();
}

Windows::Data::Json::JsonObject^ AzureUpdateModel::GetReportedProperties() const
{
    TRACE(L"AzureUpdateModel::GetReportedProperties()");

    wstring state;
    state += _manifestFileName;
    if (IsDownloaded())
    {
        if (state.length() != 0)
        {
            state += L",";
        }
        state += UpdateDownloaded;
    }
    if (IsInstalled())
    {
        if (state.length() != 0)
        {
            state += L",";
        }
        state += UpdateInstalled;
    }

    JsonObject^ updateProperties = ref new JsonObject();
    updateProperties->Insert(UpdateState, JsonValue::CreateStringValue(ref new String(state.c_str())));

    return updateProperties;
}
