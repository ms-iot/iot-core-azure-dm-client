#pragma once

#include "DMException.h"
#include <windows.h>

class SecurityAttributes
{
public:
    SecurityAttributes(DWORD permissions);
    ~SecurityAttributes();
    LPSECURITY_ATTRIBUTES GetSA();

private:
    SECURITY_ATTRIBUTES _securityAttributes;
    PSID _everyoneSID;
    PACL _ACL;
    PSECURITY_DESCRIPTOR _SD;
};