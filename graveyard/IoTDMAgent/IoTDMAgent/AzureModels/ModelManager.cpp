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
            else if (0 == WindowsUpdateModel::NodeName().compare(childKey->Data()))
            {
                _windowsUpdateModel.SetDesiredProperties(pair->Value);
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

        // Azure Update properties
        root->Insert(ref new String(AzureUpdateManager::NodeName().c_str()), _azureUpdateManager.GetReportedProperties());

        // Windows Update properties
        root->Insert(ref new String(WindowsUpdateModel::NodeName().c_str()), _windowsUpdateModel.GetReportedProperties());
    }

    string jsonString = Utils::WideToMultibyte(root->Stringify()->Data());
    TRACEP("Json = ", jsonString.c_str());

    return jsonString;
}
