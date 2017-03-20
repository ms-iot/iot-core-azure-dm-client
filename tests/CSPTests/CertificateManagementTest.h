#pragma once

#include <string>
#include <vector>

class CertificateManagementTest
{
public:
    static bool RunTest();

private:
    static void AddCertificate(const std::wstring& cspPath);
    static void DeleteCertificate(const std::wstring& cspPath);
    static void ListCertificates(const std::wstring& cspPath, std::vector<std::wstring>& hashesVector);
    static bool IsCertificatePresent(const std::wstring& cspPath);
};