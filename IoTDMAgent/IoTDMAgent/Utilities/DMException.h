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
};
