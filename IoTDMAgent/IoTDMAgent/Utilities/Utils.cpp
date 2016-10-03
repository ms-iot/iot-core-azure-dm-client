#include "stdafx.h"
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
    if (!requiredCharCount)
    {
        return string();
    }

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
    if (!requiredCharCount)
    {
        return wstring();
    }

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


time_t TimeFromSystemTime(const SYSTEMTIME& time)
{
    struct tm tm;
    memset(&tm, 0, sizeof(tm));

    tm.tm_year = time.wYear - 1900;
    tm.tm_mon = time.wMonth - 1;
    tm.tm_mday = time.wDay;

    tm.tm_hour = time.wHour;
    tm.tm_min = time.wMinute;
    tm.tm_sec = time.wSecond;

    return mktime(&tm);
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

bool StringToInt(const std::wstring& s, unsigned int& i)
{
    bool result = true;
    try
    {
        i = std::stoi(s);
    }
    catch (std::invalid_argument&)
    {
        result = false;
    }
    catch (std::out_of_range&)
    {
        result = false;
    }
    return result;
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