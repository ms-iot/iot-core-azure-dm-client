#pragma once

#include <string>
#include <map>
#include <windows.h>

// ToDo: turn this into a class like the JsonReader in the C# project.
class JsonReader
{
public:

    static bool TryFindString(
        const std::map<std::wstring, Windows::Data::Json::IJsonValue^>& properties,
        const std::wstring& path, std::wstring& stringValue);

    template<class T>
    static bool TryFindNumber(
        const std::map<std::wstring, Windows::Data::Json::IJsonValue^>& properties,
        const std::wstring& path, T& numberValue)
    {
        std::map<std::wstring, Windows::Data::Json::IJsonValue^>::const_iterator it = properties.find(path);
        if (it == properties.end())
        {
            return false;
        }

        if (it->second->ValueType != Windows::Data::Json::JsonValueType::Number)
        {
            return false;
        }

        numberValue = static_cast<T>(it->second->GetNumber());
        return true;
    }

    static bool TryFindDateTime(
        const std::map<std::wstring, Windows::Data::Json::IJsonValue^>& properties,
        const std::wstring& path, SYSTEMTIME& dateTimeValue);

    static void Flatten(
        const std::wstring& path,
        Windows::Data::Json::JsonObject^ root,
        std::map<std::wstring, Windows::Data::Json::IJsonValue^>& properties);
};