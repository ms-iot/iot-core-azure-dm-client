#include "stdafx.h"
#include <fstream>
#include <iostream> 
#include <iomanip>
#include "JsonHelpers.h"
#include "TimeHelpers.h"
#include "Utils.h"
#include "Logger.h"
#include "DMException.h"

using namespace Windows::System;
using namespace Platform;
using namespace Windows::Data::Json;
using namespace Windows::System::Profile;
using namespace Windows::Foundation::Collections;

using namespace std;
using namespace Utils;

#ifdef GetObject
#undef GetObject
#endif

bool JsonReader::TryFindString(const map<wstring, IJsonValue^>& properties, const wstring& path, wstring& stringValue)
{
    TRACE(__FUNCTION__);

    map<wstring, IJsonValue^>::const_iterator it = properties.find(path);
    if (it == properties.end())
    {
        return false;
    }

    if (it->second->ValueType != JsonValueType::String)
    {
        return false;
    }

    stringValue = it->second->GetString()->Data();
    return true;
}

bool JsonReader::TryFindDateTime(const map<wstring, IJsonValue^>& properties, const wstring& path, SYSTEMTIME& dateTimeValue)
{
    TRACE(__FUNCTION__);

    map<wstring, IJsonValue^>::const_iterator it = properties.find(path);
    if (it == properties.end())
    {
        return false;
    }

    if (it->second->ValueType != JsonValueType::String)
    {
        return false;
    }

    if (!SystemTimeFromISO8601(it->second->GetString()->Data(), dateTimeValue))
    {
        return false;
    }

    return true;
}

void JsonReader::Flatten(const wstring& path, JsonObject^ root, map<wstring, IJsonValue^>& properties)
{
    for (IIterator<IKeyValuePair<String^, IJsonValue^>^>^ iter = root->First();
        iter->HasCurrent;
        iter->MoveNext())
    {
        IKeyValuePair<String^, IJsonValue^>^ pair = iter->Current;
        String^ childKey = pair->Key;
        wstring childPath = (path.size() ? path + L"." : L"") + childKey->Data();

        properties[childPath] = pair->Value;
        if (pair->Value->ValueType == JsonValueType::Object)
        {
            Flatten(childPath, pair->Value->GetObject(), properties);
        }
    }
}