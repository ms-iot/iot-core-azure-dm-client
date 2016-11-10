#include "stdafx.h"
#include "ModelManager.h"

using namespace Windows::Data::Json;
using namespace Windows::Foundation::Collections;
using namespace Platform;
using namespace std;

#define JsonDesiredNode L"desired"

IJsonValue^ ModelManager::GetDesiredPropertiesNode(bool completeSet, const string& allJson)
{
    TRACE(__FUNCTION__);

    wstring wideJsonString = Utils::MultibyteToWide(allJson.c_str());

    JsonValue^ value;
    if (!JsonValue::TryParse(ref new String(wideJsonString.c_str()), &value) || (value == nullptr))
    {
        throw DMException("Failed to parse Json.");
    }

    if (!completeSet)
    {
        return value;
    }

    // Locate the 'desired' node.
    JsonObject^ object = value->GetObject();
    if (object == nullptr)
    {
        throw DMException("Unexpected device twin update element.");
    }

    IJsonValue^ desiredPropertiesNode = nullptr;
    for (IIterator<IKeyValuePair<String^, IJsonValue^>^>^ iter = object->First();
        iter->HasCurrent;
        iter->MoveNext())
    {
        IKeyValuePair<String^, IJsonValue^>^ pair = iter->Current;
        String^ childKey = pair->Key;

        // Look for "desired"
        if (childKey == JsonDesiredNode && pair->Value != nullptr)
        {
            desiredPropertiesNode = pair->Value;
            break;
        }
    }

    if (!desiredPropertiesNode)
    {
        throw DMException("Failed to find the desired properties node.");
    }

    return desiredPropertiesNode;
}

void ModelManager::ProcessDesiredProperties(bool completeSet, const string& allJson)
{
    TRACE(__FUNCTION__);

    IJsonValue^ desiredValue = GetDesiredPropertiesNode(completeSet, allJson);
    ProcessDesiredProperties(desiredValue);
}

void ModelManager::ProcessDesiredProperties(IJsonValue^ desiredPropertyValue)
{
    TRACE(__FUNCTION__);
    switch (desiredPropertyValue->ValueType)
    {
    case JsonValueType::Object:
    {
        // Iterate through the desired properties top-level nodes.
        JsonObject^ object = desiredPropertyValue->GetObject();
        if (object == nullptr)
        {
            TRACE("Warning: Unexpected desired properties contents. Skipping.");
            return;
        }

        for (IIterator<IKeyValuePair<String^, IJsonValue^>^>^ iter = object->First();
            iter->HasCurrent;
            iter->MoveNext())
        {
            IKeyValuePair<String^, IJsonValue^>^ pair = iter->Current;
            String^ childKey = pair->Key;

            if (0 == RebootModel::NodeName().compare(childKey->Data()))
            {
                _rebootModel.SetDesiredProperties(pair->Value);
            }
            else if (0 == AzureUpdateManager::NodeName().compare(childKey->Data()))
            {
                _azureUpdateManager.SetDesiredProperties(pair->Value);
            }
        }
    }
    break;
    }
}

string ModelManager::ExecRebootNow()
{
    TRACE(__FUNCTION__);

    return _rebootModel.ExecRebootNow();
}

string ModelManager::GetRebootPropertiesJson() const
{
    TRACE(__FUNCTION__);

    JsonObject^ root = ref new JsonObject();
    root->Insert(ref new String(RebootModel::NodeName().c_str()), _rebootModel.GetReportedProperties());

    string jsonString = Utils::WideToMultibyte(root->Stringify()->Data());
    TRACEP("Json = ", jsonString.c_str());

    return jsonString;
}

string ModelManager::ExecWipeNow()
{
    TRACE(__FUNCTION__);

    return _remoteWipeModel.ExecWipe();
}

string ModelManager::GetWipePropertiesJson() const
{
    TRACE(__FUNCTION__);

    JsonObject^ root = ref new JsonObject();
    root->Insert(ref new String(RemoteWipeModel::NodeName().c_str()), _remoteWipeModel.GetReportedProperties());

    string jsonString = Utils::WideToMultibyte(root->Stringify()->Data());
    TRACEP("Json = ", jsonString.c_str());

    return jsonString;
}

string ModelManager::GetAllPropertiesJson() const
{
    TRACE(__FUNCTION__);

    JsonObject^ root = ref new JsonObject();
    {
        // System Info properties
        root->Insert(ref new String(SystemInfoModel::NodeName().c_str()), _systemInfoModel.GetReportedProperties());

        // Time properties
        root->Insert(ref new String(TimeModel::NodeName().c_str()), _timeModel.GetReportedProperties());

        // Reboot properties
        root->Insert(ref new String(RebootModel::NodeName().c_str()), _rebootModel.GetReportedProperties());

        // Update properties
        root->Insert(ref new String(AzureUpdateManager::NodeName().c_str()), _azureUpdateManager.GetReportedProperties());
    }

    string jsonString = Utils::WideToMultibyte(root->Stringify()->Data());
    TRACEP("Json = ", jsonString.c_str());

    return jsonString;
}
