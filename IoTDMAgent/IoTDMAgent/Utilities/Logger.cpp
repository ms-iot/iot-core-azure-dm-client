#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <ostream>
#include <iomanip>
#include <sstream>
#include "Logger.h"

using namespace std;

Logger::Logger(bool console, const wchar_t* logFileName) :
    _console(console),
    _logFileName(logFileName)
{
    Log("----New Session----------------------------------------------------------------");
}

void Logger::Log(const char*  message)
{
    std::wstring s = Utils::MultibyteToWide(message);
    Log(s.c_str());
}

void Logger::Log(const wchar_t*  message)
{
    SYSTEMTIME systemTime;
    GetLocalTime(&systemTime);

    // format time
    basic_ostringstream<wchar_t> formattedTime; 
    formattedTime << setw(2) << setfill(L'0') << (systemTime.wHour > 12 ? (systemTime.wHour - 12) : systemTime.wHour) 
        << L'-' << setw(2) << setfill(L'0') << systemTime.wMinute
        << L'-' << setw(2) << setfill(L'0') << systemTime.wSecond;

    // format thread id
    basic_ostringstream<wchar_t> formattedThreadId;
    formattedThreadId << setw(8) << setfill(L'0') << GetThreadId(GetCurrentThread());

    // build message
    wstring messageWithTime = formattedTime.str() + L" "
                            + (systemTime.wHour >= 12 ? L"PM " : L"AM ")
                            + L"[" + formattedThreadId.str() + L"] "
                            + message
                            + L"\r\n";

    // share...
    if (_console)
    {
        wcout << messageWithTime;
    }

    if (_logFileName.size())
    {
        basic_ofstream<wchar_t> outFile(_logFileName, fstream::app);
        outFile << messageWithTime;
        outFile.close();
    }
}

void Logger::Log(const char*  format, const char* param)
{
    std::wstring f = Utils::MultibyteToWide(format);
    std::wstring p = Utils::MultibyteToWide(param);
    Log<const wchar_t*>(f.c_str(), p.c_str());
}

void Logger::Log(const char*  format, int param)
{
    std::wstring f = Utils::MultibyteToWide(format);
    Log<int>(f.c_str(), param);
}