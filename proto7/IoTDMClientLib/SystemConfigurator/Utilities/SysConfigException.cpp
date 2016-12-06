#include "stdafx.h"
#include "Windows.h" 
#include "SysConfigException.h"

sysconfig_exception::sysconfig_exception() :
   _error(GetLastError())
{
}
