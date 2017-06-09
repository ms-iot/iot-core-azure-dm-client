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
#include <vector>
#include <map>
#include "DMException.h"
#include "PermissionsSnapshot.h"
#include "PermissionsTracer.h"

using namespace std;

PermissionsSnapshot::PermissionsSnapshot(const std::wstring& objectName, SE_OBJECT_TYPE objectType, SECURITY_INFORMATION securityInformation) :
    _objectName(objectName),
    _objectType(objectType),
    _capturedFlags(securityInformation),
    _ownerSid(nullptr),
    _groupSid(nullptr),
    _DACL(nullptr),
    _SACL(nullptr),
    _securityDescriptor(nullptr)
{
    TRACEP(L"Capture security info for ", _objectName.c_str());

    DWORD result = GetNamedSecurityInfo(
        const_cast<wchar_t*>(objectName.c_str()),
        objectType,
        securityInformation,
        &_ownerSid,
        &_groupSid,
        &_DACL,
        &_SACL,
        &_securityDescriptor);

    if (result != ERROR_SUCCESS)
    {
        throw DMExceptionWithErrorCode("Error: GetNamedSecurityInfo(). Error Code = ", result);
    }

    PermissionsTracer::TraceSID(L"............Owner SID = ", _ownerSid);
    PermissionsTracer::TraceSID(L"............Group SID = ", _groupSid);
    PermissionsTracer::TraceSD(objectName, _securityDescriptor);
}

void PermissionsSnapshot::Apply()
{
    TRACEP(L"Restoring security info for ", _objectName.c_str());

    LPWSTR objNamePtr = const_cast<wchar_t*>(_objectName.c_str());

    DWORD result = SetNamedSecurityInfo(objNamePtr, _objectType, _capturedFlags, _ownerSid, _groupSid, _DACL, _SACL);
    if (ERROR_SUCCESS != result)
    {
        throw DMExceptionWithErrorCode("Error: SetNamedSecurityInfo(). Error Code = ", result);
    }
}

PSID PermissionsSnapshot::OwnerSid()
{
    return _ownerSid;
}

PSID PermissionsSnapshot::GroupSid()
{
    return _groupSid;
}

PACL PermissionsSnapshot::DACL()
{
    return _DACL;
}

PACL PermissionsSnapshot::SACL()
{
    return _SACL;
}

PSECURITY_DESCRIPTOR PermissionsSnapshot::SecurityDescriptor()
{
    return _securityDescriptor;
}