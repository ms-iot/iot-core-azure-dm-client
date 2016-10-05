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

    std::wstring GetResourceString(int id);

    void ReadXmlValue(const std::wstring& resultSyncML, const std::wstring& targetXmlPath, std::wstring& value);
}