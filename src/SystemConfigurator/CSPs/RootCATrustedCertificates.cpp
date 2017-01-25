#include "stdafx.h"
#include "RootCATrustedCertificatesCSP.h"
#include "MdmProvision.h"
#include "..\SharedUtilities\Logger.h"

using namespace std;

const wchar_t* RootCATrstedCertificatesPath = L"./Device/Vendor/MSFT/RootCATrstedCertificates";

// RootCATrstedCertificates CSP docs
// https://msdn.microsoft.com/en-us/windows/hardware/commercialize/customize/mdm/rootcacertificates-csp
//

wstring RootCATrustedCertificatesCSP::GetCertHashes(const wstring& subPath)
{
    TRACE(__FUNCTION__);

    wstring path = RootCATrstedCertificatesPath;
    path += L"/";
    path += subPath;

    wstring hashes = MdmProvision::RunGetString(path);
    TRACEP(L" hashes = ", hashes.c_str());
    return hashes;
}

void RootCATrustedCertificatesCSP::AddCertificate(const wstring& subPath, const wstring& certHash, const wstring& certificateInBase64)
{
    TRACE(__FUNCTION__);

    wstring path = RootCATrstedCertificatesPath;
    path += L"/";
    path += subPath;
    path += L"/";
    path += certHash;
    path += L"/EncodedCertificate";

    MdmProvision::RunAddData(path, certificateInBase64);
}

void RootCATrustedCertificatesCSP::DeleteCertificate(const wstring& subPath, const wstring& certHash)
{
    TRACE(__FUNCTION__);

    TRACE(__FUNCTION__);

    wstring path = RootCATrstedCertificatesPath;
    path += L"/";
    path += subPath;
    path += L"/";
    path += certHash;

    MdmProvision::RunDelete(path);
}

CertificateInfo RootCATrustedCertificatesCSP::GetCertificateInfo(const wstring& subPath, const wstring& certHash)
{
    TRACE(__FUNCTION__);

    wstring path = RootCATrstedCertificatesPath;
    path += L"/";
    path += subPath;
    path += L"/";
    path += certHash;

    return CertificateInfo(path);
}