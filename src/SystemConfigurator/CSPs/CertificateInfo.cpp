#include "stdafx.h"
#include "CertificateInfo.h"
#include "MdmProvision.h"
#include "..\SharedUtilities\Logger.h"

using namespace std;

CertificateInfo::CertificateInfo(const std::wstring& certPath) :
    _certPath(certPath)
{
    TRACE(__FUNCTION__);
}

std::wstring CertificateInfo::GetCertificateInBase64()
{
    TRACE(__FUNCTION__);

    return MdmProvision::RunGetString(_certPath + L"/EncodedCertificate");
};

void CertificateInfo::SetCertificateInBase64(const std::wstring& certPath, const std::wstring& certificateInBase64)
{
    TRACE(__FUNCTION__);

    wstring path = certPath + L"/EncodedCertificate";

    MdmProvision::RunSetBase64(path, certificateInBase64);
}

void CertificateInfo::AddCertificateInBase64(const std::wstring& certPath, const std::wstring& certificateInBase64)
{
    TRACE(__FUNCTION__);

    wstring path = certPath + L"/EncodedCertificate";

    MdmProvision::RunAddData(path, certificateInBase64);
}

std::wstring CertificateInfo::GetIssuedBy()
{
    TRACE(__FUNCTION__);

    return MdmProvision::RunGetString(_certPath + L"/IssuedBy");
}

void CertificateInfo::SetIssuedBy(const std::wstring& certPath, const std::wstring& value)
{
    TRACE(__FUNCTION__);

    wstring path = certPath + L"/IssuedBy";

    MdmProvision::RunSetBase64(path, value);
}

void CertificateInfo::AddIssuedBy(const std::wstring& certPath, const std::wstring& value)
{
    TRACE(__FUNCTION__);

    wstring path = certPath + L"/IssuedBy";

    MdmProvision::RunAddData(path, value);
}

std::wstring CertificateInfo::GetIssuedTo()
{
    TRACE(__FUNCTION__);

    return MdmProvision::RunGetString(_certPath + L"/IssuedTo");
}

void CertificateInfo::SetIssuedTo(const std::wstring& certPath, const std::wstring& value)
{
    TRACE(__FUNCTION__);

    wstring path = certPath + L"/IssuedTo";

    MdmProvision::RunSetBase64(path, value);
}

void CertificateInfo::AddIssuedTo(const std::wstring& certPath, const std::wstring& value)
{
    TRACE(__FUNCTION__);

    wstring path = certPath + L"/IssuedTo";

    MdmProvision::RunAddData(path, value);
}

std::wstring CertificateInfo::GetValidFrom()
{
    TRACE(__FUNCTION__);

    return MdmProvision::RunGetString(_certPath + L"/ValidFrom");
}

void CertificateInfo::SetValidFrom(const std::wstring& certPath, const std::wstring& value)
{
    TRACE(__FUNCTION__);

    wstring path = certPath + L"/ValidFrom";

    MdmProvision::RunSetBase64(path, value);
}

void CertificateInfo::AddValidFrom(const std::wstring& certPath, const std::wstring& value)
{
    TRACE(__FUNCTION__);

    wstring path = certPath + L"/ValidFrom";

    MdmProvision::RunAddData(path, value);
}

std::wstring CertificateInfo::GetValidTo()
{
    TRACE(__FUNCTION__);

    return MdmProvision::RunGetString(_certPath + L"/ValidTo");
}

void CertificateInfo::SetValidTo(const std::wstring& certPath, const std::wstring& value)
{
    TRACE(__FUNCTION__);

    wstring path = certPath + L"/ValidTo";

    MdmProvision::RunSetBase64(path, value);
}

void CertificateInfo::AddValidTo(const std::wstring& certPath, const std::wstring& value)
{
    TRACE(__FUNCTION__);

    wstring path = certPath + L"/ValidTo";

    MdmProvision::RunAddData(path, value);
}

std::wstring CertificateInfo::GetTemplateName()
{
    TRACE(__FUNCTION__);

    return MdmProvision::RunGetString(_certPath + L"/TemplateName");
}

void CertificateInfo::SetTemplateName(const std::wstring& certPath, const std::wstring& value)
{
    TRACE(__FUNCTION__);

    wstring path = certPath + L"/TemplateName";

    MdmProvision::RunSetBase64(path, value);
}

void CertificateInfo::AddTemplateName(const std::wstring& certPath, const std::wstring& value)
{
    TRACE(__FUNCTION__);

    wstring path = certPath + L"/TemplateName";

    MdmProvision::RunAddData(path, value);
}

void CertificateInfo::AddCertificate(const std::wstring& path, const std::wstring& hash, const std::wstring& certificateInBase64)
{
    TRACE(__FUNCTION__);

    wstring fullPath = path;
    fullPath += L"/";
    fullPath += hash;
    fullPath += L"/EncodedCertificate";

    TRACEP(L"Adding : ", fullPath.c_str());

    MdmProvision::RunAddData(fullPath, certificateInBase64);
}

void CertificateInfo::DeleteCertificate(const wstring& path, const wstring& hash)
{
    TRACE(__FUNCTION__);

    wstring fullPath = path;
    fullPath += L"/";
    fullPath += hash;

    TRACEP(L"Deleting : ", fullPath.c_str());

    MdmProvision::RunDelete(path);
}