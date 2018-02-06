/*
Copyright 2017 Microsoft
Permission is hereby granted, free of charge, to any person obtaining a copy of this software 
and associated documentation files (the "Software"), to deal in the Software without restriction, 
including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, 
subject to the following conditions:

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT 
LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH 
THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "stdafx.h"
#include <windows.h>
#include <fstream>
#include <iostream> 
#include <iomanip>
#include "StringUtils.h"
#include "Logger.h"
#include "ETWLogger.h"

using namespace std;

Utils::ETWLogger gETWLogger;

Logger::Logger(bool console) :
    _console(console)
{
    Log("----New Session----------------------------------------------------------------");
}

void Logger::Log(const char* msg)
{
    wstring s = Utils::MultibyteToWide(msg);
    Log(Utils::ETWLogger::LoggingLevel::Information, s.c_str());
}

void Logger::Log(const wchar_t* msg)
{
    Log(Utils::ETWLogger::LoggingLevel::Information, msg);
}

void Logger::Log(Utils::ETWLogger::LoggingLevel level, const wchar_t* msg)
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
        + msg
        + L"\r\n";

    // share...
    if (_console)
    {
        lock_guard<mutex> guard(_mutex);
        wcout << messageWithTime;
    }

    if (_logFileName.size() != 0)
    {
        lock_guard<mutex> guard(_mutex);

        wofstream logFile(_logFileName.c_str(), std::ofstream::out | std::ofstream::app);
        if (logFile)
        {
            logFile << messageWithTime.c_str();
            logFile.close();
        }
    }

    gETWLogger.Log(msg, level);
}

void Logger::Log(const char* msg, const char* param)
{
    Log(Utils::ETWLogger::LoggingLevel::Information, msg, param);
}

void Logger::Log(Utils::ETWLogger::LoggingLevel level, const char*  msg, const char* param)
{
    wstring m = Utils::MultibyteToWide(msg);
    wstring p = Utils::MultibyteToWide(param);
    Log<const wchar_t*>(level, m.c_str(), p.c_str());

}

void Logger::Log(const char*  msg, int param)
{
    wstring s = Utils::MultibyteToWide(msg);
    Log<int>(Utils::ETWLogger::LoggingLevel::Information, s.c_str(), param);
}

void Logger::Log(Utils::ETWLogger::LoggingLevel level, const char*  msg, int param)
{
    wstring s = Utils::MultibyteToWide(msg);
    Log<int>(level, s.c_str(), param);
}

void Logger::SetLogFileName(const wstring& logFileName)
{
    _logFileName = logFileName;
}