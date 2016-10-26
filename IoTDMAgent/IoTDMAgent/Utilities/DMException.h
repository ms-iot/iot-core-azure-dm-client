#pragma once

#include <exception>
#include "Logger.h"

class DMException : public std::exception
{
public:
    DMException(const char* message) :
        std::exception(message)
    {
        TRACEP("Exception: ", message);
    }

    template<class T>
    DMException(const char* message, T parameter) :
        std::exception(message)
    {
        basic_ostringstream<char> messageString;
        messageString << message << " " << parameter;

        TRACEP("Exception: ", messageString.str().c_str());
    }
};
