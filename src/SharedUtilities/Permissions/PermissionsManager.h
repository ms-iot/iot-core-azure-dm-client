#pragma once
#include <AccCtrl.h>
#include <string>
#include <functional>

class PermissionsManager
{
public:
    static void ModifyProtected(const std::wstring& objectName, SE_OBJECT_TYPE objType, const std::function<void()>& Action);
private:
    static HANDLE OpenProcessToken(DWORD desiredAccess);
};