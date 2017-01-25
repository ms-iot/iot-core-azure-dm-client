#pragma once

#include <string>

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

private:
    std::wstring _certPath;
};