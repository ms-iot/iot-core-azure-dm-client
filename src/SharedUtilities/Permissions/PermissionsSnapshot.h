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
#pragma once
#include <windows.h>
#include <Aclapi.h>
#include <Sddl.h>
#include <string>

class PermissionsSnapshot
{
    enum Component
    {
        Unknown,
        Owner,
        Group,
        DaclFlags,
        SaclFlags,
    };

public:
    PermissionsSnapshot(const std::wstring& objectName, SE_OBJECT_TYPE objectType, SECURITY_INFORMATION securityInformation);

    void Apply();

    PSID OwnerSid();
    PSID GroupSid();
    PACL DACL();
    PACL SACL();
    PSECURITY_DESCRIPTOR SecurityDescriptor();

private:

    std::wstring _objectName;
    SE_OBJECT_TYPE _objectType;

    SECURITY_INFORMATION _capturedFlags;

    PSID _ownerSid;
    PSID _groupSid;
    PACL _DACL;
    PACL _SACL;
    PSECURITY_DESCRIPTOR  _securityDescriptor;
};