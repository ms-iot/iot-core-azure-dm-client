#include "stdafx.h"
#include <windows.h>
#include <Aclapi.h>
#include <Sddl.h>
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include "utils.h"
#include "DMException.h"
#include "PermissionsManager.h"
#include "PermissionsSnapshot.h"

using namespace std;

PSID BuildSID(SID_IDENTIFIER_AUTHORITY authority, BYTE subAuthorityCount, DWORD subAuthority0, DWORD subAuthority1)
{
    TRACE(__FUNCTION__);

    PSID sid = NULL;
    if (!AllocateAndInitializeSid(
        &authority,
        subAuthorityCount,
        subAuthority0, subAuthority1, 0, 0, 0, 0, 0, 0,
        &sid))
    {
        throw DMExceptionWithErrorCode("Error: AllocateAndInitializeSid(). Error Code = ", GetLastError());
    }
    return sid;
}

PACL BuildDACL(PSID newOwner, PSID everyoneSID)
{
    TRACE(__FUNCTION__);

    const unsigned int NUM_ACES = 2;
    EXPLICIT_ACCESS ea[NUM_ACES];
    ZeroMemory(&ea, NUM_ACES * sizeof(EXPLICIT_ACCESS));

    // Set read access for Everyone.
    ea[0].grfAccessPermissions = GENERIC_READ;
    ea[0].grfAccessMode = SET_ACCESS;
    ea[0].grfInheritance = NO_INHERITANCE;
    ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea[0].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
    ea[0].Trustee.ptstrName = (LPTSTR)everyoneSID;

    // Set full control for Administrators.
    ea[1].grfAccessPermissions = GENERIC_ALL;
    ea[1].grfAccessMode = SET_ACCESS;
    ea[1].grfInheritance = NO_INHERITANCE;
    ea[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea[1].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
    ea[1].Trustee.ptstrName = (LPTSTR)newOwner;

    PACL newDACL = NULL;
    if (ERROR_SUCCESS != SetEntriesInAcl(NUM_ACES, ea, NULL, &newDACL))
    {
        throw DMExceptionWithErrorCode("Error: SetEntriesInAcl(). Error Code = ", GetLastError());
    }
    return newDACL;
}

void SetPrivilege(HANDLE processToken, LPCTSTR privilegeName, BOOL enable)
{
    TRACE(__FUNCTION__);

    LUID luid;
    if (!LookupPrivilegeValue(NULL /*local*/, privilegeName, &luid))
    {
        throw DMExceptionWithErrorCode("Error: LookupPrivilegeValue(). Error Code = ", GetLastError());
    }

    TOKEN_PRIVILEGES tp;
    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = enable ? SE_PRIVILEGE_ENABLED : 0;
    if (!AdjustTokenPrivileges(processToken, FALSE /*disable*/, &tp, sizeof(TOKEN_PRIVILEGES), NULL /*prev state*/, NULL /*prev state len*/))
    {
        throw DMExceptionWithErrorCode("Error: AdjustTokenPrivileges(). Error Code = ", GetLastError());
    }
    if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
    {
        throw DMException("Error: AdjustTokenPrivileges(). The processToken does not have the specified privilege.");
    }
}

void SetNamedSecurityInfo(const wstring& objName, SE_OBJECT_TYPE objType, SECURITY_INFORMATION securityInfo, PSID owner, PACL dacl)
{
    TRACE(__FUNCTION__);

    LPWSTR objNamePtr = const_cast<wchar_t*>(objName.c_str());

    DWORD retCode = SetNamedSecurityInfo(objNamePtr, objType, securityInfo, owner, NULL /*group*/, dacl, NULL /*sacl*/);
    if (ERROR_SUCCESS != retCode)
    {
        throw DMExceptionWithErrorCode("Error: SetNamedSecurityInfo(). Error Code = ", retCode);
    }
}

HANDLE PermissionsManager::OpenProcessToken(DWORD desiredAccess)
{
    TRACE(__FUNCTION__);

    HANDLE token = NULL;
    if (!::OpenProcessToken(GetCurrentProcess(), desiredAccess, &token))
    {
        throw DMExceptionWithErrorCode("Error: OpenProcessToken(). Error Code = ", GetLastError());
    }
    return token;
}

void PermissionsManager::ModifyProtected(const std::wstring& objectName, SE_OBJECT_TYPE objType, const std::function<void()>& Action)
{
    TRACE(__FUNCTION__);

    // BUILTIN\Administrators group
    Utils::AutoCloseSID newOwner = BuildSID(SECURITY_NT_AUTHORITY, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS);
    // Everyone group
    Utils::AutoCloseSID everyoneSID = BuildSID(SECURITY_WORLD_SID_AUTHORITY, 1, SECURITY_WORLD_RID, 0);
    Utils::AutoCloseACL newDACL = BuildDACL(newOwner.Get(), everyoneSID.Get());
    Utils::AutoCloseHandle processToken = OpenProcessToken(TOKEN_ADJUST_PRIVILEGES);

    wchar_t* requiredPrivileges[] = { SE_TAKE_OWNERSHIP_NAME, SE_BACKUP_NAME, SE_RESTORE_NAME, SE_SECURITY_NAME, NULL };
    for (size_t i = 0; requiredPrivileges[i] != NULL; ++i)
    {
        SetPrivilege(processToken.Get(), requiredPrivileges[i], TRUE);
    }

    PermissionsSnapshot originalState(objectName, objType, OWNER_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION);

    try
    {
        TRACE(L"Taking ownership...");
        SetNamedSecurityInfo(objectName, objType, OWNER_SECURITY_INFORMATION, newOwner.Get(), NULL);

        SetPrivilege(processToken.Get(), SE_TAKE_OWNERSHIP_NAME, FALSE);

        TRACE(L"Setting new dacl...");
        SetNamedSecurityInfo(objectName, objType, DACL_SECURITY_INFORMATION, NULL, newDACL.Get());

        Action();
    }
    catch (const DMExceptionWithErrorCode&)
    {
        originalState.Apply();
        throw;
    }

    originalState.Apply();
}
