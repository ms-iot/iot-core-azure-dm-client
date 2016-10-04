#pragma once

#include <Objidl.h>
#include <string>

class XmlParser
{
public:
    static void ReadXmlValue(const std::wstring& resultSyncML, const std::wstring& targetXmlPath, std::wstring& value);

private:
    static void ReadXmlValue(IStream* resultSyncML, const std::wstring& targetXmlPath, std::wstring& value);
};