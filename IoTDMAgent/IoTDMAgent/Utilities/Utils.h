#pragma once

#include <stdio.h>
#include <string>
#include <vector>
#include <time.h>

namespace Utils
{
    std::string WideToMultibyte(const wchar_t* s);
    std::wstring MultibyteToWide(const char* s);

    void SplitString(const std::wstring& s, wchar_t delim, std::vector<std::wstring>& tokens);

    std::wstring GetCurrentDateTimeString();
    std::wstring GetDateTimeString(unsigned int year, unsigned int month, unsigned int day, unsigned int hour, unsigned int minute, unsigned int second);

    std::wstring GetResourceString(int id);

    void ReadXmlValue(const std::wstring& resultSyncML, const std::wstring& targetXmlPath, std::wstring& value);

    void WriteRegistryValue(const std::wstring& subkey, const std::wstring& propName, const std::wstring& propValue);
    std::wstring ReadRegistryValue(const std::wstring& subkey, const std::wstring& propName);

    std::wstring GetOSVersionString();
}