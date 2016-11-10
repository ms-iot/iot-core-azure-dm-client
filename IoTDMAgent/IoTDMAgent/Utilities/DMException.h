#pragma once

#include <exception>
#include "Logger.h"

typedef long HRESULT;

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

class DMExceptionWithErrorCode : public DMException
{
    int _errorcode;
public:
    DMExceptionWithErrorCode(int errorcode) :
        DMException(""), _errorcode(errorcode)
    {
        TRACEP("Exception error code: ", errorcode);
    }
};

class DMExceptionWithHRESULT : public DMException
{
    HRESULT _hr;
public:
    DMExceptionWithHRESULT(HRESULT hr) :
        DMException(""), _hr(hr)
    {
        TRACEP("Exception HRESULT: ", hr);
    }
};