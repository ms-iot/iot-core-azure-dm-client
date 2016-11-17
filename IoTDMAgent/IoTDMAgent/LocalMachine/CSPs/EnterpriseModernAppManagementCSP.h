#pragma once

#include <string>
#include <vector>

class EnterpriseModernAppManagementCSP
{
public:
    static std::vector<std::wstring> GetInstalledApps();
    static void InstallApp(const std::wstring& packageFamilyName, const std::wstring& packageUri, const std::vector<std::wstring>& dependentPackages);
    static void UninstallApp(const std::wstring& packageFamilyName);
};