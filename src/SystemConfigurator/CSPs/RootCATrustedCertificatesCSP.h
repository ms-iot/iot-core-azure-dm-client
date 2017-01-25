#pragma once

#include <string>
#include "CertificateInfo.h"

class RootCATrustedCertificatesCSP
{
public:
    // subPath : Root, CA, TrustedPublisher, TrustedPeople
    static std::wstring GetCertHashes(const std::wstring& subPath);

    static void AddCertificate(const std::wstring& subPath, const std::wstring& certHash, const std::wstring& certificateInBase64);
    static void DeleteCertificate(const std::wstring& subPath, const std::wstring& certHash);

    static CertificateInfo GetCertificateInfo(const std::wstring& subPath, const std::wstring& certHash);

};