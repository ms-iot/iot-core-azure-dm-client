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
//
// Temporary code - will be eventually replaced.
//
#pragma once
#include <strsafe.h>
#include "IProvisioningDPULocal.h"

// Name of host variable to retrieve the session's AccountID from within a CSP
#define OMADM_ACCOUNTID_VARIABLE_NAME       TEXT("OMADM::AccountID")

// Name of host variable to retrieve the session's Targeted User SID from within a CSP
#define OMADM_TARGETEDUSERSID_VARIABLE_NAME TEXT("OMADM::TargetedUserSID")

// Error codes
#define OMADM_ERROR_ALREADY_EXISTS 418 

HRESULT MdmProvisionSyncBodyWithAttributes(
    _In_                   PCWSTR  pszInContent,
    _In_opt_               PCWSTR  pszServerID,
                           DWORD   cVariables,
    _In_reads_(cVariables) SYNCMLATTRIBUTE* rgVariables,
    _Outptr_               PWSTR* ppszOutContent
);
