#pragma once

#include <Windows.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <time.h>

namespace Utils
{
    std::string WideToMultibyte(const wchar_t* s);
    std::wstring MultibyteToWide(const char* s);

    time_t TimeFromSystemTime(const SYSTEMTIME& time);

    void SplitString(const std::wstring& s, wchar_t c, std::vector<std::wstring>& tokens);

    bool StringToInt(const std::wstring& s, unsigned int& i);
}