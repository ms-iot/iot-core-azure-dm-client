#pragma once

#include <string>
#include <vector>

namespace Utils
{
    std::string WideToMultibyte(const wchar_t* s);
    std::wstring MultibyteToWide(const char* s);

    template<class T>
    void SplitString(const std::basic_string<T> &s, T delim, std::vector<std::basic_string<T>>& tokens)
    {
        std::basic_stringstream<T> ss;
        ss.str(s);
        std::basic_string<T> item;
        while (getline<T>(ss, item, delim))
        {
            tokens.push_back(item);
        }
    }

    template<class T>
    T TrimString(const T& s, const T& chars)
    {
        T trimmedString;

        // trim leading characters
        size_t startpos = s.find_first_not_of(chars);
        if (T::npos != startpos)
        {
            trimmedString = s.substr(startpos);
        }

        // trim trailing characters
        size_t endpos = trimmedString.find_last_not_of(chars);
        if (T::npos != endpos)
        {
            trimmedString = trimmedString.substr(0, endpos + 1);
        }
        return trimmedString;
    }

    std::wstring TrimString(const std::wstring& s, const std::wstring& suffix);

    template<class CharType, class ParamType>
    std::basic_string<CharType> ConcatString(const CharType* s, ParamType param)
    {
        std::basic_ostringstream<CharType> messageStream;
        messageStream << s << param;
        return messageStream.str();
    }
}