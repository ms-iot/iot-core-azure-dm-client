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