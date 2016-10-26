#include "stdafx.h"
#include <filesystem>
#include "AzureUpdateManager.h"
#include "..\Utilities\Logger.h"
#include "..\Utilities\AzureStorage.h"

using namespace Windows::Data::Json;
using namespace Windows::Foundation::Collections;
using namespace Platform;
using namespace std;
using namespace tr2::sys;

#define AzureUpdateNode L"azureUpdates"
#define AzureStorageConnStr L"connStr"
#define AzureContainerName L"container"
#define ManifestsNode L"manifests"

#define LocalUpdatesFolder L"c:\\temp\\updates"

AzureUpdateManager::AzureUpdateManager()
{
    TRACE("AzureUpdateManager::AzureUpdateManager()");
    LoadLocalState(LocalUpdatesFolder, AzureManifestsFolder);
}

wstring AzureUpdateManager::NodeName()
{
    return AzureUpdateNode;
}

void AzureUpdateManager::SetDesiredOperations(Windows::Data::Json::IJsonValue^ cabsNode, vector<UpdateOperation>& desiredOperations)
{
    TRACE(L"AzureUpdateManager::SetDesiredOperations()");

    JsonValueType type = cabsNode->ValueType;
    if (type != JsonValueType::Object)
    {
        TRACE("Warning: unexpected desired operation object type.");
        return;
    }

    JsonObject^ object = cabsNode->GetObject();
    if (object == nullptr)
    {
        TRACE("Warning: unexpected desired operation null object.");
        return;
    }

    for (IIterator<IKeyValuePair<String^, IJsonValue^>^>^ iter = object->First();
        iter->HasCurrent;
        iter->MoveNext())
    {
        IKeyValuePair<String^, IJsonValue^>^ pair = iter->Current;
        UpdateOperation operation;

        if (pair->Value->ValueType != JsonValueType::String)
        {
            TRACE("Warning: unexpected desired operation value type.");
            continue;
        }

        wstring cabOperationsString = pair->Value->GetString()->Data();
        vector<wstring> tokens;
        Utils::SplitString(cabOperationsString, L',', tokens);
        size_t index = 0;
        for (const wstring& s : tokens)
        {
            if (index == 0)
            {
                operation.manifestFileName = s;
            }
            else if (s == L"apply")
            {
                operation.apply = true;
            }
            else if (s == L"download")
            {
                operation.download = true;
            }
            ++index;
        }
        desiredOperations.push_back(operation);
    }
}

void AzureUpdateManager::LoadLocalState(const std::wstring& updatesLocalRoot, const std::wstring& manifestsFolder)
{
    TRACE(L"AzureUpdateManager::LoadLocalState()");

    lock_guard<mutex> lock(_mutex);

    // Make sure the update engine has the most up-to-date list of installed packages.
    _updateEngine.Scan();

    // Start a fresh list
    _localUpdates.clear();

    // This folder is expected to have only manifest files.
    path p(updatesLocalRoot + L"\\" + manifestsFolder);
    for (auto it = directory_iterator(p); it != directory_iterator(); it++)
    {
        if (is_directory(it->path()))
        {
            continue;
        }

        wstring manifestName = Utils::MultibyteToWide(it->path().filename().string().c_str());
        TRACEP(L"Found: ", manifestName.c_str());

        _localUpdates[manifestName] = shared_ptr<AzureUpdateModel>(new AzureUpdateModel(updatesLocalRoot, manifestName, _updateEngine));
    }
}

void AzureUpdateManager::ExecuteDesiredOperations(const wstring& connectionString, const wstring& containerName, const vector<UpdateOperation>& desiredOperations)
{
    TRACE(L"AzureUpdateManager::ExecuteDesiredOperations()");

    lock_guard<mutex> lock(_mutex);

    for (const UpdateOperation& operation : desiredOperations)
    {
        TRACE(L"For each operation...");
        map<wstring, shared_ptr<AzureUpdateModel>>::iterator it = _localUpdates.find(operation.manifestFileName);
        if (it == _localUpdates.end())
        {
            TRACE(L"Reference to new cab...");
            shared_ptr<AzureUpdateModel> azureUpdateModel(new AzureUpdateModel(LocalUpdatesFolder, operation.manifestFileName, _updateEngine));
            _localUpdates[operation.manifestFileName] = azureUpdateModel;
            it = _localUpdates.find(operation.manifestFileName);
        }

        AzureUpdateModel& azureUpdateModel = *(it->second);

        if (operation.download)
        {
            TRACE(L"Operation = download");
            if (!azureUpdateModel.IsDownloaded())
            {
                TRACE(L"It is not already downloaded...");
                azureUpdateModel.Download(connectionString);
            }
        }

        if (operation.apply)
        {
            TRACE(L"Operation = apply");
            if (!azureUpdateModel.IsInstalled())
            {
                TRACE(L"It is not already applied...");
                azureUpdateModel.Install();
            }
        }
    }
}

void AzureUpdateManager::SetDesiredProperties(Windows::Data::Json::IJsonValue^ azureUpdatesNode)
{
    TRACE(L"AzureUpdateManager::SetDesiredProperties()");

    wstring connectionString;
    wstring containerName;

    JsonValueType type = azureUpdatesNode->ValueType;
    if (type != JsonValueType::Object)
    {
        TRACE("Warning: Unexpected reboot properties contents. Skipping...");
        return;
    }

    JsonObject^ object = azureUpdatesNode->GetObject();
    if (object == nullptr)
    {
        TRACE("Warning: Unexpected reboot properties contents. Skipping...");
        return;
    }

    vector<UpdateOperation> operations;
    for (IIterator<IKeyValuePair<String^, IJsonValue^>^>^ iter = object->First();
        iter->HasCurrent;
        iter->MoveNext())
    {
        IKeyValuePair<String^, IJsonValue^>^ pair = iter->Current;
        String^ childKey = pair->Key;
        if (childKey == AzureStorageConnStr)
        {
            if (pair->Value->ValueType == JsonValueType::String)
            {
                connectionString = pair->Value->GetString()->Data();
            }
        }
        else if (childKey == AzureContainerName)
        {
            if (pair->Value->ValueType == JsonValueType::String)
            {
                containerName = pair->Value->GetString()->Data();
            }
        }
        else if (childKey == ManifestsNode)
        {
            if (pair->Value->ValueType == JsonValueType::Object)
            {
                SetDesiredOperations(pair->Value, operations /*out*/);
            }
        }
    }

    // Now that we have read all the properties, we can execute the operations...
    ExecuteDesiredOperations(connectionString, containerName, operations);
}

Windows::Data::Json::JsonObject^ AzureUpdateManager::GetReportedProperties() const
{
    TRACE(L"AzureUpdateManager::GetReportedProperties()");

    lock_guard<mutex> lock(_mutex);

    JsonObject^ updateProperties = ref new JsonObject();
    for (const auto& p : _localUpdates)
    {
        String^ manifestId = ref new String(Utils::ToJsonPropoertyName(p.first).c_str());
        updateProperties->Insert(manifestId, p.second->GetReportedProperties());
    }

    string jsonString = Utils::WideToMultibyte(updateProperties->Stringify()->Data());
    TRACEP("Update Model Json = ", jsonString.c_str());

    return updateProperties;
}