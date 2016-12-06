#pragma once

#include <exception>

class sysconfig_exception : public std::exception
{
public:
    sysconfig_exception();
private:
    int _error;
};
