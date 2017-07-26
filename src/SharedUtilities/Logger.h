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
#pragma once

#include <mutex>
#include <string>
#include <sstream>
#include "ETWLogger.h"

#include "..\DMMessage\DMGarbageCollectorTempFolder.h"

class Logger
{
public:
    Logger(bool console);

    // Legacy (no logging level; defaults to information)
    void Log(const char*  message);
    void Log(const wchar_t*  message);

    template<class T>
    void Log(const wchar_t* msg, T param)
    {
        std::basic_ostringstream<wchar_t> message;
        message << msg << param;
        Log(message.str().c_str());
    }

    void Log(const char* msg, const char* param);
    void Log(const char* msg, int param);

    // Logging level set...
    void Log(Utils::ETWLogger::LoggingLevel level, const char*  msg);
    void Log(Utils::ETWLogger::LoggingLevel level, const wchar_t*  msg);

    template<class T>
    void Log(Utils::ETWLogger::LoggingLevel level, const wchar_t* msg, T param)
    {
        std::basic_ostringstream<wchar_t> message;
        message << msg << param;
        Log(level, message.str().c_str());
    }

    void Log(Utils::ETWLogger::LoggingLevel level, const char* msg, const char* param);
    void Log(Utils::ETWLogger::LoggingLevel level, const char* msg, int param);

private:
    std::mutex _mutex;

    bool _console;
};

Logger __declspec(selectany) gLogger(true /*console output*/);

#define TRACE(msg) gLogger.Log(msg)
#define TRACEP(format, param) gLogger.Log(format, param)
