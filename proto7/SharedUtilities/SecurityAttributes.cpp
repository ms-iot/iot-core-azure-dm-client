#include "stdafx.h"
#include <Aclapi.h>
#include <iostream>
#include "SecurityAttributes.h"

using namespace std;

SecurityAttributes::SecurityAttributes(DWORD permissions) : _everyoneSID(nullptr), _ACL(nullptr), _SD(nullptr)
{
    SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;

    // Create a well-known SID for the Everyone group.
    if (!AllocateAndInitializeSid(&SIDAuthWorld, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, &_everyoneSID))
    {
        wcout << L"AllocateAndInitializeSid Error: " << GetLastError() << endl;
        return;
    }

    EXPLICIT_ACCESS ea = { 0 };
    ea.grfAccessPermissions = permissions;
    ea.grfAccessMode = SET_ACCESS;
    ea.grfInheritance = NO_INHERITANCE;
    ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
    ea.Trustee.ptstrName = (LPTSTR)_everyoneSID;

    // Create a new ACL that contains the new ACEs.
    DWORD dwRes = SetEntriesInAclW(1, &ea, NULL, &_ACL);
    if (dwRes != ERROR_SUCCESS)
    {
        wcout << L"SetEntriesInAcl Error: " << GetLastError() << endl;
        throw sysconfig_exception();
    }

    // Initialize a security descriptor.  
    _SD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
    if (_SD == NULL)
    {
        wcout << L"LocalAlloc Error: " << GetLastError() << endl;
        throw sysconfig_exception();
    }

    if (!InitializeSecurityDescriptor(_SD, SECURITY_DESCRIPTOR_REVISION))
    {
        wcout << L"InitializeSecurityDescriptor Error: " << GetLastError() << endl;
        throw sysconfig_exception();
    }

    // Add the ACL to the security descriptor. 
    if (!SetSecurityDescriptorDacl(_SD, TRUE, _ACL, FALSE))
    {
        wcout << L"SetSecurityDescriptorDacl Error: " << GetLastError() << endl;
        throw sysconfig_exception();
    }

    // Initialize a security attributes structure.
    _securityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
    _securityAttributes.lpSecurityDescriptor = _SD;
    _securityAttributes.bInheritHandle = FALSE;
}

SecurityAttributes::~SecurityAttributes()
{
    if (_everyoneSID)
        FreeSid(_everyoneSID);
    if (_ACL)
        LocalFree(_ACL);
    if (_SD)
        LocalFree(_SD);
}

LPSECURITY_ATTRIBUTES SecurityAttributes::GetSA()
{
    return &_securityAttributes;
}
