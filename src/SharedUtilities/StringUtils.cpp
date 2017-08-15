#include "stdafx.h"
#include <windows.h>
#include "StringUtils.h"

using namespace std;

namespace Utils
{
    string WideToMultibyte(const wchar_t* s)
    {
        size_t length = s ? wcslen(s) : 0;
        size_t requiredCharCount = WideCharToMultiByte(CP_UTF8, 0, s, static_cast<int>(length), nullptr, 0, nullptr, nullptr);

        // add room for \0
        ++requiredCharCount;

        vector<char> multibyteString(requiredCharCount);
        WideCharToMultiByte(CP_UTF8, 0, s, static_cast<int>(length), multibyteString.data(), static_cast<int>(multibyteString.size()), nullptr, nullptr);

        return string(multibyteString.data());
    }

    wstring MultibyteToWide(const char* s)
    {
        size_t length = s ? strlen(s) : 0;
        size_t requiredCharCount = MultiByteToWideChar(CP_UTF8, 0, s, static_cast<int>(length), nullptr, 0);

        // add room for \0
        ++requiredCharCount;

        vector<wchar_t> wideString(requiredCharCount);
        MultiByteToWideChar(CP_UTF8, 0, s, static_cast<int>(length), wideString.data(), static_cast<int>(wideString.size()));

        return wstring(wideString.data());
    }

    wstring TrimString(const std::wstring& s, const std::wstring& suffix)
    {
        wstring trimmed = s;

        size_t pos = s.find(suffix);
        if (wstring::npos != pos && pos == s.length() - suffix.length())
        {
            trimmed = s.substr(0, s.length() - suffix.length());
        }
        return trimmed;
    }

    bool Contains(const wstring& container, const wstring& contained)
    {
        if (container.size() < contained.size())
        {
            return false;
        }

        bool match = false;
        for (size_t i = 0; (i < container.size() - contained.size() + 1) && !match; ++i)
        {
            match = true;
            for (size_t j = 0; j < contained.size(); ++j)
            {
                if (towlower(container[i + j]) != towlower(contained[j]))
                {
                    match = false;
                    break;
                }
            }
        }
        return match;
    }
}