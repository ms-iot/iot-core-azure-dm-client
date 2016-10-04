#include "stdafx.h"
#include <windows.h>
#include <time.h>
#include <string>
#include <vector>
#include <sstream>
#include <codecvt>
#include "Utils.h"
#include "..\resource.h"

using namespace std;

namespace Utils
{

std::string WideToMultibyte(const wchar_t* s)
{
    size_t length = 0;
    if (s == nullptr || (0 == (length = wcslen(s))))
    {
        return string();
    }

    size_t requiredCharCount = WideCharToMultiByte(CP_UTF8, 0, s, static_cast<int>(length), nullptr, 0, nullptr, nullptr);

    // add room for \0
    ++requiredCharCount;

    vector<char> multibyteString(requiredCharCount);
    requiredCharCount = WideCharToMultiByte(CP_UTF8, 0, s, static_cast<int>(length), multibyteString.data(), static_cast<int>(multibyteString.size()), nullptr, nullptr);
    if (!requiredCharCount)
    {
        return string();
    }

    return string(multibyteString.data());
}

std::wstring MultibyteToWide(const char* s)
{
    size_t length = 0;
    if (s == nullptr || (0 == (length = strlen(s))))
    {
        return wstring();
    }

    size_t requiredCharCount = MultiByteToWideChar(CP_UTF8, 0, s, static_cast<int>(length), nullptr, 0);

    // add room for \0
    ++requiredCharCount;

    vector<wchar_t> wideString(requiredCharCount);
    requiredCharCount = MultiByteToWideChar(CP_UTF8, 0, s, static_cast<int>(length), wideString.data(), static_cast<int>(wideString.size()));
    if (!requiredCharCount)
    {
        return wstring();
    }

    return wstring(wideString.data());
}

void SplitString(const wstring &s, wchar_t delim, vector<wstring> &tokens) {
    basic_stringstream<wchar_t> ss;
    ss.str(s);
    wstring item;
    while (getline<wchar_t>(ss, item, delim))
    {
        tokens.push_back(item);
    }
}

wstring GetResourceString(int id)
{
    const int bufferSize = 1024;
    wchar_t buffer[bufferSize];
    if (!LoadString(GetModuleHandle(NULL), id, buffer, bufferSize))
    {
        if (!LoadString(GetModuleHandle(NULL), IDS_OMA_SYNCML_STATUS_UNKNOWN, buffer, bufferSize))
        {
            return wstring(L"Unknown OMA SyncML status code.");
        }
    }
    return wstring(buffer);
}

}