#pragma once

#include <Objidl.h>
#include <string>

class XmlParser
{
public:
    static bool ReadXmlValue(const std::wstring& resultSyncML, const std::wstring& targetXmlPath, std::wstring& value);

private:
    static bool ReadXmlValue(IStream* resultSyncML, const std::wstring& targetXmlPath, std::wstring& value);
};