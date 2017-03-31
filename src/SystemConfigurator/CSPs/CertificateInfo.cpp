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

    MdmProvision::RunSet(path, value);
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

    MdmProvision::RunSet(path, value);
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

    MdmProvision::RunSet(path, value);
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

    MdmProvision::RunSet(path, value);
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

    MdmProvision::RunSet(path, value);
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
