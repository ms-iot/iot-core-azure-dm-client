#pragma once

#include <string>

namespace Utils
{
    class ETWLogger
    {
    public:
        enum LoggingLevel
        {
            Verbose = 0,
            Information = 1,
            Warning = 2,
            Error = 3,
            Critical = 4
        };

        ETWLogger();
        ~ETWLogger();
        void Log(const std::wstring& msg, LoggingLevel level);
        void Log(const std::string& msg, LoggingLevel level);

    private:
        std::wstring GetExeFileName();

        std::wstring _exeFileName;
    };
}