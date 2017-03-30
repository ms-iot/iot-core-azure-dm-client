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
#include "Utils.h"

#include "..\DMMessage\DMGarbageCollectorTempFolder.h"
#define TRACE_LOGSROOT SC_CLEANUP_FOLDER

#define TRACE_MAX_LEN 512

class Logger
{
public:
    Logger(bool console, const wchar_t* logsRoot);

    void Log(const char*  message);
    void Log(const wchar_t*  message);

    template<class T>
    void Log(const wchar_t* format, T param)
    {
        std::basic_ostringstream<wchar_t> message;
        message << format << param;
        Log(message.str().c_str());
    }

    void Log(const char* format, const char* param);
    void Log(const char* format, int param);

private:
    std::mutex _mutex;

    bool _console;
    std::wstring _logFileName;
};

#ifdef _DEBUG
Logger __declspec(selectany) gLogger(true /*console output*/, TRACE_LOGSROOT);

#define TRACE(msg) gLogger.Log(msg)
#define TRACEP(format, param) gLogger.Log(format, param)
#else
#define TRACE(msg)
#define TRACEP(format, param)
#endif
