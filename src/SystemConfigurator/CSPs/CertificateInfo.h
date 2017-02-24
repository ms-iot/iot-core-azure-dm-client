#pragma once

#include <string>
#include <sstream>
#include <iomanip>
#include "..\SharedUtilities\Utils.h"
#include "..\SharedUtilities\DMException.h"

class CertificateInfo
{
public:
    CertificateInfo(const std::wstring& certPath);

    std::wstring GetCertificateInBase64();
    static void SetCertificateInBase64(const std::wstring& certPath, const std::wstring& certificateInBase64);
    static void AddCertificateInBase64(const std::wstring& certPath, const std::wstring& certificateInBase64);

    std::wstring GetIssuedBy();
    static void SetIssuedBy(const std::wstring& certPath, const std::wstring& value);
    static void AddIssuedBy(const std::wstring& certPath, const std::wstring& value);

    std::wstring GetIssuedTo();
    static void SetIssuedTo(const std::wstring& certPath, const std::wstring& value);
    static void AddIssuedTo(const std::wstring& certPath, const std::wstring& value);

    std::wstring GetValidFrom();
    static void SetValidFrom(const std::wstring& certPath, const std::wstring& value);
    static void AddValidFrom(const std::wstring& certPath, const std::wstring& value);

    std::wstring GetValidTo();
    static void SetValidTo(const std::wstring& certPath, const std::wstring& value);
    static void AddValidTo(const std::wstring& certPath, const std::wstring& value);

    std::wstring GetTemplateName();
    static void SetTemplateName(const std::wstring& certPath, const std::wstring& value);
    static void AddTemplateName(const std::wstring& certPath, const std::wstring& value);

    static void AddCertificate(const std::wstring& path, const std::wstring& hash, const std::wstring& certificateInBase64);
    static void DeleteCertificate(const std::wstring& path, const std::wstring& hash);

private:
    std::wstring _certPath;
};

class CertificateFileInfo
{
public:
    CertificateFileInfo(const std::wstring& certFileName)
    {
        std::vector<char> certEncoded;
        Utils::LoadFile(certFileName, certEncoded);

        PCCERT_CONTEXT certContext = CertCreateCertificateContext(X509_ASN_ENCODING, reinterpret_cast<BYTE*>(certEncoded.data()), certEncoded.size());
        if (!certContext)
        {
            throw new DMException("Error: CertCreateCertificateContext() failed.", GetLastError());
        }

        DWORD thumbPrintSize = 512;
        std::vector<BYTE> thumbPrint(thumbPrintSize);

        BOOL result = CryptHashCertificate(
            NULL, // Not used.
            0,    // algorithm id. Default is 0 (SHA1).
            0,    // flags to be passed to the hash API.
            certContext->pbCertEncoded,
            certContext->cbCertEncoded,
            thumbPrint.data(),
            &thumbPrintSize);

        if (result)
        {
            std::basic_ostringstream<wchar_t> thumbPrintString;
            for (unsigned int i = 0; i < thumbPrintSize; ++i)
            {
                thumbPrintString << std::setw(2) << std::setfill(L'0') << std::hex << thumbPrint[i];
            }
            _thumbPrint = thumbPrintString.str();
        }

        CertFreeCertificateContext(certContext);

        if (!result)
        {
            throw new DMException("Error: CryptHashCertificate() failed.", GetLastError());
        }

        _fullFileName = certFileName;
    }

    std::wstring FullFileName() const
    {
        return _fullFileName;
    }

    std::wstring ThumbPrint() const
    {
        return _thumbPrint;
    }
private:
    std::wstring _fullFileName;
    std::wstring _thumbPrint;
};