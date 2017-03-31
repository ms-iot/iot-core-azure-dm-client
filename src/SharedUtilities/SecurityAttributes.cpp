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
        throw DMExceptionWithErrorCode("SetEntriesInAcl Error", GetLastError());
    }

    // Initialize a security descriptor.  
    _SD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
    if (_SD == NULL)
    {
        throw DMExceptionWithErrorCode("LocalAlloc Error", GetLastError());
    }

    if (!InitializeSecurityDescriptor(_SD, SECURITY_DESCRIPTOR_REVISION))
    {
        throw DMExceptionWithErrorCode("InitializeSecurityDescriptor Error", GetLastError());
    }

    // Add the ACL to the security descriptor. 
    if (!SetSecurityDescriptorDacl(_SD, TRUE, _ACL, FALSE))
    {
        throw DMExceptionWithErrorCode("SetSecurityDescriptorDacl Error", GetLastError());
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
