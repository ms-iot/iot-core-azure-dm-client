#include "stdafx.h"
#include <windows.h>
#include <fstream>
#include <iostream> 
#include <iomanip>
#include <filesystem>
#include "Logger.h"

using namespace std;
using namespace std::tr2::sys;
using namespace std::experimental;

Logger::Logger(bool console, const wchar_t* logsRoot) :
    _console(console)
{
    if (!filesystem::exists(logsRoot))
    {
        error_code code;
        filesystem::create_directory(logsRoot, code);
    }
	
	basic_ostringstream<wchar_t> fileName;
    fileName << logsRoot;

    wchar_t moduleFileName[MAX_PATH] = { 0 };
    DWORD length = GetModuleFileName(NULL, moduleFileName, sizeof(moduleFileName) / sizeof(moduleFileName[0]));
    if (length != 0 && length != MAX_PATH)
    {
        path p(moduleFileName);
        fileName << p.filename();
        fileName << L".";
    }

    fileName << GetCurrentProcessId();
    fileName << LOGFILE_EXT;

    _logFileName = fileName.str();
    Log("----New Session----------------------------------------------------------------");
}

void Logger::Log(const char* message)
{
    wstring s = Utils::MultibyteToWide(message);
    Log(s.c_str());
}

void Logger::Log(const wchar_t* message)
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
        lock_guard<mutex> guard(_mutex);
        wcout << messageWithTime;
    }

    if (_logFileName.size())
    {
        lock_guard<mutex> guard(_mutex);
        basic_ofstream<wchar_t> outFile(_logFileName, fstream::app);
        outFile << messageWithTime;
        outFile.close();
    }
}

void Logger::Log(const char*  format, const char* param)
{
    wstring f = Utils::MultibyteToWide(format);
    wstring p = Utils::MultibyteToWide(param);
    Log<const wchar_t*>(f.c_str(), p.c_str());
}

void Logger::Log(const char*  format, int param)
{
    wstring f = Utils::MultibyteToWide(format);
    Log<int>(f.c_str(), param);
}