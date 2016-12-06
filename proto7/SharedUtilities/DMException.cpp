#include "stdafx.h"
#include <windows.h>
#include "DMException.h"

sysconfig_exception::sysconfig_exception() :
    _error(GetLastError())
{
}
