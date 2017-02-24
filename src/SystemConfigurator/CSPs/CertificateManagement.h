#pragma once

class CertificateManagement
{
public:
    static void SyncCertificates(const std::wstring& path, const std::wstring& desiredCertificateFiles);
};