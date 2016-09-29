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
    SYSTEMTIME systemTime;
    GetLocalTime(&systemTime);

    // format time
    ostringstream formattedTime; 
    formattedTime << setw(2) << setfill('0') << (systemTime.wHour > 12 ? (systemTime.wHour - 12) : systemTime.wHour) 
        << '-' << setw(2) << setfill('0') << systemTime.wMinute
        << '-' << setw(2) << setfill('0') << systemTime.wSecond;

    // format thread id
    ostringstream formattedThreadId;
    formattedThreadId << setw(8) << setfill('0') << GetThreadId(GetCurrentThread());

    // build message
    string messageWithTime = formattedTime.str() + " "
                            + (systemTime.wHour >= 12 ? "PM " : "AM ")
                            + "[" + formattedThreadId.str() + "] "
                            + message
                            + "\r\n";

    // share...
    if (_console)
    {
        cout << messageWithTime;
    }

    if (_logFileName.size())
    {
        ofstream outFile(_logFileName, fstream::app);
        outFile << messageWithTime;
        outFile.close();
    }
}

void Logger::Log(const wchar_t*  message)
{
    std::string s = Utils::WideToMultibyte(message);
    Log(s.c_str());
}

void Logger::Log(const wchar_t*  format, const wchar_t* param)
{
    std::string m = Utils::WideToMultibyte(format);
    std::string p = Utils::WideToMultibyte(param);
    Log<const char*>(m.c_str(), p.c_str());
}

void Logger::Log(const wchar_t*  format, int param)
{
    std::string m = Utils::WideToMultibyte(format);
    Log<int>(m.c_str(), param);
}