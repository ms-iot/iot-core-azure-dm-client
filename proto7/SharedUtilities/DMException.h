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
    int _errorCode;
public:
    DMExceptionWithErrorCode(int errorCode) :
        DMException(""), _errorCode(errorCode)
    {
        TRACEP("Exception error code: ", errorCode);
    }

    DMExceptionWithErrorCode(const char* message, int errorCode) :
        DMException(message), _errorCode(errorCode)
    {
        TRACEP("Exception error code: ", errorCode);
    }

    int ErrorCode() const
    {
        return _errorCode;
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

    DMExceptionWithHRESULT(const char* message, HRESULT hr) :
        DMException(message), _hr(hr)
    {
        TRACEP("Exception HRESULT: ", hr);
    }
};