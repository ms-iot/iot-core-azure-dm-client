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

// ToDo: locking?!
#define UpdateDownloaded L"downloaded"
#define UpdateInstalled L"installed"
#define UpdateState L"state"
#define StagingFolder L"C:\\Data\\ProgramData\\USOShared\\Data"

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

        // update the applied state
        _cabsInstalled = VerifyCabsInstalled(_cabsData, updateEngine);
    }
}

void AzureUpdateModel::ParseManifest(const wstring& updatesLocalRoot, const wstring& manifestFileName, std::vector<CabData>& cabsData)
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

bool AzureUpdateModel::VerifyCabsInstalled(const std::vector<CabData> cabsData, const UpdateEngine& updateEngine)
{
    TRACE(L"AzureUpdateModel::VerifyInstalled()");

    bool applied = true;
    for (const CabData& cabData : cabsData)
    {
        applied = updateEngine.IsInstalled(cabData.id, cabData.version);
        if (!applied)
        {
            break;
        }
    }

    if (applied)
    {
        TRACE(L"All cabs are installed.");
    }
    else
    {
        TRACE(L"Not all cabs are installed.");
    }

    return applied;
}

void AzureUpdateModel::Download(const std::wstring& connectionString)
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
    TRACE(L"AzureUpdateModel::Apply()");

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