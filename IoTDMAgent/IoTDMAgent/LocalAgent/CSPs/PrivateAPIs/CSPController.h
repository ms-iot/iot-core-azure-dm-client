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

HRESULT MdmProvisionSyncBodyWithAttributes(
    _In_                   PCWSTR  pszInContent,
    _In_opt_               PCWSTR  pszServerID,
                           DWORD   cVariables,
    _In_reads_(cVariables) SYNCMLATTRIBUTE* rgVariables,
    _Outptr_               PWSTR* ppszOutContent
);
